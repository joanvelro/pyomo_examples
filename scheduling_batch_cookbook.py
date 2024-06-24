import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd

import shutil
import sys
import os.path

from pyomo.environ import *
from pyomo.gdp import *

"""
https://jckantor.github.io/ND-Pyomo-Cookbook/notebooks/04.03-Job-Shop-Scheduling.html
"""


def build_model(TASKS):
    model = ConcreteModel()

    # tasks is a two dimensional set of (j,m) constructed from the dictionary keys
    model.TASKS = Set(initialize=TASKS.keys(), dimen=2)

    # the set of jobs is constructed from a python set
    model.JOBS = Set(initialize=list(set([j for (j, m) in model.TASKS])))

    # set of machines is constructed from a python set
    model.MACHINES = Set(initialize=list(set([m for (j, m) in model.TASKS])))

    # the order of tasks is constructed as a cross-product of tasks and filtering
    model.TASKORDER = Set(initialize=model.TASKS * model.TASKS, dimen=4,
                          filter=lambda model, j, m, k, n: (k, n) == TASKS[(j, m)]['prec'])

    # the set of disjunctions is cross-product of jobs, jobs, and machines
    model.DISJUNCTIONS = Set(initialize=model.JOBS * model.JOBS * model.MACHINES, dimen=3,
                             filter=lambda model, j, k, m: j < k and (j, m) in model.TASKS and (k, m) in model.TASKS)

    # load duration data into a model parameter for later access
    model.dur = Param(model.TASKS, initialize=lambda model, j, m: TASKS[(j, m)]['dur'])

    # establish an upper bound on makespan
    ub = sum([model.dur[j, m] for (j, m) in model.TASKS])

    # create decision variables
    model.makespan = Var(bounds=(0, ub))
    model.start = Var(model.TASKS, bounds=(0, ub))

    model.objective = Objective(expr=model.makespan, sense=minimize)

    model.finish = Constraint(model.TASKS, rule=lambda model, j, m:
    model.start[j, m] + model.dur[j, m] <= model.makespan)

    model.preceding = Constraint(model.TASKORDER, rule=lambda model, j, m, k, n:
    model.start[k, n] + model.dur[k, n] <= model.start[j, m])

    model.disjunctions = Disjunction(model.DISJUNCTIONS, rule=lambda model, j, k, m:
    [model.start[j, m] + model.dur[j, m] <= model.start[k, m],
     model.start[k, m] + model.dur[k, m] <= model.start[j, m]])

    TransformationFactory('gdp.hull').apply_to(model)
    return model


def build_model_clean(tasks: dict, tclean: float = 0):
    TASKS = tasks
    model = ConcreteModel()

    # tasks is a two-dimensional set of (j,m) constructed from the dictionary keys
    model.TASKS = Set(initialize=TASKS.keys(), dimen=2)

    # the set of jobs is constructed from a python set
    model.JOBS = Set(initialize=list(set([j for (j, m) in model.TASKS])))

    # set of machines is constructed from a python set
    model.MACHINES = Set(initialize=list(set([m for (j, m) in model.TASKS])))

    # the order of tasks is constructed as a cross-product of tasks and filtering
    model.TASKORDER = Set(initialize=model.TASKS * model.TASKS, dimen=4,
                          filter=lambda model, j, m, k, n: (k, n) == TASKS[(j, m)]['prec'])

    # the set of disjunctions is cross-product of jobs, jobs, and machines
    model.DISJUNCTIONS = Set(initialize=model.JOBS * model.JOBS * model.MACHINES, dimen=3,
                             filter=lambda model, j, k, m: j < k and (j, m) in model.TASKS and (k, m) in model.TASKS)

    # load duration data into a model parameter for later access
    model.dur = Param(model.TASKS, initialize=lambda model, j, m: TASKS[(j, m)]['dur'])

    # establish an upper bound on makespan
    ub = sum([model.dur[j, m] for (j, m) in model.TASKS])

    model.makespan = Var(bounds=(0, ub))

    model.start = Var(model.TASKS, bounds=(0, ub))

    model.objective = Objective(expr=model.makespan, sense=minimize)

    model.finish = Constraint(model.TASKS, rule=lambda model, j, m:
    model.start[j, m] + model.dur[j, m] <= model.makespan)

    model.preceding = Constraint(model.TASKORDER, rule=lambda model, j, m, k, n:
    model.start[k, n] + model.dur[k, n] <= model.start[j, m])

    model.disjunctions = Disjunction(model.DISJUNCTIONS, rule=lambda model, j, k, m:
    [model.start[j, m] + model.dur[j, m] + tclean <= model.start[k, m],
     model.start[k, m] + model.dur[k, m] + tclean <= model.start[j, m]])

    TransformationFactory('gdp.hull').apply_to(model)

    return model


def jobshop_solve(model, solver: str = 'cbc', verbosity: bool = True) -> list:
    solver_ = SolverFactory(solver)
    solver_.solve(model, tee=verbosity)
    results = [{'Job': j,
                'Machine': m,
                'Start': model.start[j, m](),
                'Duration': model.dur[j, m],
                'Finish': model.start[(j, m)]() + model.dur[j, m]}
               for j, m in model.TASKS]
    return results


def jobshop(TASKS):
    return jobshop_solve(build_model(TASKS))


def visualize(results: list) -> None:
    schedule = pd.DataFrame(results)
    JOBS = sorted(list(schedule['Job'].unique()))
    MACHINES = sorted(list(schedule['Machine'].unique()))
    makespan = schedule['Finish'].max()

    bar_style = {'alpha': 1.0, 'lw': 25, 'solid_capstyle': 'butt'}
    text_style = {'color': 'white', 'weight': 'bold', 'ha': 'center', 'va': 'center'}
    colors = mpl.cm.Dark2.colors

    schedule.sort_values(by=['Job', 'Start'])
    schedule.set_index(['Job', 'Machine'], inplace=True)

    fig, ax = plt.subplots(2, 1, figsize=(12, 5 + (len(JOBS) + len(MACHINES)) / 4))

    for jdx, j in enumerate(JOBS, 1):
        for mdx, m in enumerate(MACHINES, 1):
            if (j, m) in schedule.index:
                xs = schedule.loc[(j, m), 'Start']
                xf = schedule.loc[(j, m), 'Finish']
                ax[0].plot([xs, xf], [jdx] * 2, c=colors[mdx % 7], **bar_style)
                ax[0].text((xs + xf) / 2, jdx, m, **text_style)
                ax[1].plot([xs, xf], [mdx] * 2, c=colors[jdx % 7], **bar_style)
                ax[1].text((xs + xf) / 2, mdx, j, **text_style)

    ax[0].set_title('Job Schedule')
    ax[0].set_ylabel('Job')
    ax[1].set_title('Machine Schedule')
    ax[1].set_ylabel('Machine')

    for idx, s in enumerate([JOBS, MACHINES]):
        ax[idx].set_ylim(0.5, len(s) + 0.5)
        ax[idx].set_yticks(range(1, 1 + len(s)))
        ax[idx].set_yticklabels(s)
        ax[idx].text(makespan, ax[idx].get_ylim()[0] - 0.2, "{0:0.1f}".format(makespan), ha='center', va='top')
        ax[idx].plot([makespan] * 2, ax[idx].get_ylim(), 'r--')
        ax[idx].set_xlabel('Time')
        ax[idx].grid(True)

    fig.tight_layout()
    plt.show()


def recipe_to_tasks(jobs, machines, durations):
    TASKS = {}
    for j in jobs:
        prec = (None, None)
        for m, d in zip(machines, durations):
            task = (j, m)
            if prec == (None, None):
                TASKS.update({(j, m): {'dur': d, 'prec': None}})
            else:
                TASKS.update({(j, m): {'dur': d, 'prec': prec}})
            prec = task
    return TASKS


recipeA = recipe_to_tasks(jobs='A', machines=['Mixer', 'Reactor', 'Separator', 'Packaging'], durations=[1, 5, 4, 1.5])
recipeB = recipe_to_tasks(jobs='B', machines=['Separator', 'Packaging'], durations=[4.5, 1])
recipeC = recipe_to_tasks(jobs='C', machines=['Separator', 'Reactor', 'Packaging'], durations=[5, 3, 1.5])

TASKS = recipe_to_tasks(jobs=['A1', 'A2', 'A3', 'A4'],
                        machines=['Mixer', 'Reactor', 'Separator', 'Packaging'],
                        durations=[1, 5, 4, 1.5])

TASKS_2 = recipe_to_tasks(jobs=['A1', 'A2'], machines=['Mixer', 'Reactor', 'Separator', 'Packaging'],
                              durations=[1, 5, 4, 1.5])
TASKS_2.update(recipe_to_tasks(jobs=['B1', 'B2'], machines=['Separator', 'Packaging'],
                               durations=[4.5, 1]))
TASKS_2.update(recipe_to_tasks(jobs=['C1', 'C2'], machines=['Separator', 'Reactor', 'Packaging'],
                               durations=[5, 3, 1.5]))

model_instance = build_model_clean(tasks=TASKS_2, tclean=0.5)
results = jobshop_solve(model=model_instance)
visualize(results=results)
print("Makespan =", max([task['Finish'] for task in results]))
