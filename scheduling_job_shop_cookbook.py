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


def build_model(tasks):
    model = ConcreteModel()

    # load duration data into a model parameter for later access
    model.param_duration = Param(model.tasks,
                                 initialize=lambda model, j, m: tasks[(j, m)][DURATION])

    # establish an upper bound on makespan
    param_upper_bound = sum([model.param_duration[j, m] for (j, m) in model.tasks])

    # product/job j in machine m
    # tasks is a two dimensional set of (j,m) constructed from the dictionary keys
    model.tasks_set = Set(initialize=tasks.keys(), dimen=2)

    # the set of jobs is constructed from a python set
    model.jobs_set = Set(initialize=list(set([j for (j, m) in model.tasks])))

    # set of machines is constructed from a python set
    model.equipments_set = Set(initialize=list(set([m for (j, m) in model.tasks])))

    # the order of tasks is constructed as a cross-product of tasks and filtering
    model.tasks_order = Set(initialize=model.tasks_set * model.tasks_set, dimen=4,
                          filter=lambda model, j, m, k, n: (k, n) == tasks[(j, m)][PRECEDENCE])

    # the set of disjunctions is cross-product of jobs_set, jobs_set, and equipments_set
    model.disjunctions_set = Set(initialize=model.jobs_set * model.jobs_set * model.equipments_set, dimen=3,
                             filter=lambda model, j, k, m: j < k and (j, m) in model.tasks_set and (k, m) in model.tasks_set)

    # create decision variables

    model.start = Var(model.tasks_set, bounds=(0, param_upper_bound))

    model.makespan = Var(bounds=(0, param_upper_bound))

    model.finish = Constraint(model.tasks_set, rule=lambda model, j, m:
    model.start[j, m] + model.param_duration[j, m] <= model.makespan)

    # (k,n) --> (j,m)
    def rule_constraint_preceding(model, j, m, k, n):
        return model.start[k, n] + model.param_duration[k, n] <= model.start[j, m]
    model.preceding = Constraint(model.tasks_order, rule=rule_constraint_preceding)

    def rule_constraint_disjunctions(model, job_j, job_k, equipment_m):
        return [model.start[job_j, equipment_m] + model.param_duration[job_j, equipment_m] <= model.start[job_k, equipment_m],
               model.start[job_k, equipment_m] + model.param_duration[job_k, equipment_m] <= model.start[job_j, equipment_m]]
    model.disjunctions = Disjunction(model.disjunctions_set, rule= rule_constraint_disjunctions)

    model.objective = Objective(expr=model.makespan, sense=minimize)

    TransformationFactory('gdp.hull').apply_to(model)

    return model


def solve(model, solver: str = 'cbc', verbosity: bool = True) -> list:
    solver_ = SolverFactory(solver)
    solver_.solve(model, tee=verbosity)
    results = [{'Job': j,
                'Machine': m,
                'Start': model.start[j, m](),
                'Duration': model.param_duration[j, m],
                'Finish': model.start[(j, m)]() + model.param_duration[j, m]}
               for j, m in model.tasks]
    return results


def visualize(results: list) -> None:
    schedule = pd.DataFrame(results)
    jobs_set = sorted(list(schedule['Job'].unique()))
    equipments_set = sorted(list(schedule['Machine'].unique()))
    makespan = schedule['Finish'].max()

    bar_style = {'alpha': 1.0, 'lw': 25, 'solid_capstyle': 'butt'}
    text_style = {'color': 'white', 'weight': 'bold', 'ha': 'center', 'va': 'center'}
    colors = mpl.cm.Dark2.colors

    schedule.sort_values(by=['Job', 'Start'])
    schedule.set_index(['Job', 'Machine'], inplace=True)

    fig, ax = plt.subplots(2, 1, figsize=(12, 5 + (len(jobs_set) + len(equipments_set)) / 4))

    for jdx, j in enumerate(jobs_set, 1):
        for mdx, m in enumerate(equipments_set, 1):
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

    for idx, s in enumerate([jobs_set, equipments_set]):
        ax[idx].set_ylim(0.5, len(s) + 0.5)
        ax[idx].set_yticks(range(1, 1 + len(s)))
        ax[idx].set_yticklabels(s)
        ax[idx].text(makespan, ax[idx].get_ylim()[0] - 0.2, "{0:0.1f}".format(makespan), ha='center', va='top')
        ax[idx].plot([makespan] * 2, ax[idx].get_ylim(), 'r--')
        ax[idx].set_xlabel('Time')
        ax[idx].grid(True)

    fig.tight_layout()
    plt.show()


DURATION = 'Duration'
PRECEDENCE = 'Precedence'
tasks = {
    ('Product-1', 'Equipment-1'): {DURATION: 45, PRECEDENCE: None},
    ('Product-1', 'Equipment-2'): {DURATION: 10, PRECEDENCE: ('Product-1', 'Equipment-1')},
    ('Product-1', 'Equipment-4'): {DURATION: 17, PRECEDENCE: ('Product-1', 'Equipment-2')},
    ('Product-2', 'Equipment-1'): {DURATION: 20, PRECEDENCE: ('Product-3', 'Equipment-2')},
    ('Product-2', 'Equipment-2'): {DURATION: 12, PRECEDENCE: ('Product-2', 'Equipment-1')},
    ('Product-2', 'Equipment-3'): {DURATION: 34, PRECEDENCE: ('Product-2', 'Equipment-2')},
    ('Product-2', 'Equipment-4'): {DURATION: 34, PRECEDENCE: ('Product-2', 'Equipment-3')},
    ('Product-3', 'Equipment-1'): {DURATION: 10, PRECEDENCE: None},
    ('Product-3', 'Equipment-4'): {DURATION: 28, PRECEDENCE: ('Product-3', 'Equipment-1')},
}

model_instance = build_model(tasks=tasks)
results = solve(model=model_instance)
visualize(results=results)

print("Makespan =", max([task['Finish'] for task in results]))
