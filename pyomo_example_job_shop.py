import logging
import shutil
import sys
import os.path
from pyomo.util.infeasible import log_infeasible_constraints
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import pyomo.environ as pyo
from pyomo.gdp import Disjunction
from pandas import DataFrame
import matplotlib.pyplot as plt
import matplotlib as mpl

from utils import initialize_logger

path_logger = os.path.join(os.path.dirname(__file__), '..//reports//logs//')
logger = initialize_logger(path_logger + 'pyomo_example_job_shop')

TASKS = {
    ('operation_1', 'product_A'): {'dur': 1545, 'prec': None},
    ('operation_1', 'product_B'): {'dur': 1440, 'prec': ('operation_1', 'product_A')},
    ('operation_2', 'product_A'): {'dur': 2520, 'prec': ('operation_1', 'product_B')},
    ('operation_2', 'product_C'): {'dur': 1021, 'prec': ('operation_1', 'product_B')},
    ('operation_2', 'product_B'): {'dur': 3412, 'prec': ('operation_1', 'product_A')},
    ('operation_3', 'product_A'): {'dur': 1332, 'prec': ('operation_1', 'product_B')},
    ('operation_3', 'product_C'): {'dur': 1732, 'prec': ('operation_2', 'product_A')},
    ('operation_3', 'product_B'): {'dur': 2532, 'prec': ('operation_1', 'product_A')},
}


def model_jobshop(TASKS):
    model = pyo.ConcreteModel()

    # tasks is a two dimensional set of (j,m) constructed from the dictionary keys
    model.TASKS = pyo.Set(initialize=TASKS.keys(), dimen=2)

    # the set of jobs is constructed from a python set
    model.JOBS = pyo.Set(initialize=list(set([j for (j, m) in model.TASKS])))

    # set of machines is constructed from a python set
    model.MACHINES = pyo.Set(initialize=list(set([m for (j, m) in model.TASKS])))

    # the order of tasks is constructed as a cross-product of tasks and filtering
    model.TASKORDER = pyo.Set(initialize=model.TASKS * model.TASKS, dimen=4,
                              filter=lambda model, j, m, k, n: (k, n) == TASKS[(j, m)]['prec'])

    # the set of disjunctions is cross-product of jobs, jobs, and machines
    model.DISJUNCTIONS = pyo.Set(initialize=model.JOBS * model.JOBS * model.MACHINES, dimen=3,
                                 filter=lambda model, j, k, m: j < k and (j, m) in model.TASKS and (
                                     k, m) in model.TASKS)

    # load duration data into a model parameter for later access
    model.dur = pyo.Param(model.TASKS, initialize=lambda model, j, m: TASKS[(j, m)]['dur'])

    # establish an upper bound on makespan
    ub = sum([model.dur[j, m] for (j, m) in model.TASKS])
    offset = 1
    # makespan_upper_limit = 8500  # ub*offset
    makespan_upper_limit = ub * offset
    # create decision variables
    model.makespan = pyo.Var(bounds=(0, makespan_upper_limit))
    model.start = pyo.Var(model.TASKS, bounds=(0, makespan_upper_limit))

    model.objective = pyo.Objective(expr=model.makespan, sense=pyo.minimize)

    model.finish = pyo.Constraint(model.TASKS, rule=lambda model, j, m:
    model.start[j, m] + model.dur[j, m] <= model.makespan)

    model.preceding = pyo.Constraint(model.TASKORDER, rule=lambda model, j, m, k, n:
    model.start[k, n] + model.dur[k, n] <= model.start[j, m])

    # model.constraint_makespan = pyo.Constraint(expr=model.makespan <= 8500)

    model.disjunctions = Disjunction(model.DISJUNCTIONS,
                                     rule=lambda model, j, k, m:
                                     [model.start[j, m] + model.dur[j, m] <= model.start[k, m],
                                      model.start[k, m] + model.dur[k, m] <= model.start[j, m]])

    pyo.TransformationFactory('gdp.hull').apply_to(model)
    return model


def solve_jobshop(model):
    return SolverFactory('cbc').solve(model, tee=True)


def create_and_solve_jobshop(model):
    return solve_jobshop(model)


def build_solution(model):
    schedules_dicts = [{'Job': j,
                        'Machine': m,
                        'Start': model.start[j, m](),
                        'Duration': model.dur[j, m],
                        'Finish': model.start[(j, m)]() + model.dur[j, m]}
                       for j, m in model.TASKS]
    return schedules_dicts


def has_solution(results, model):
    """
    Check the solution
    """
    status = results.solver.status
    condition = results.solver.termination_condition

    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    if not solution:
        logger.error(results)
        logger.error('Something is wrong -> status {s} and condition {c}'.format(s=status, c=condition))
        if condition == pyo.TerminationCondition.infeasible:
            log_infeasible_constraints(model,
                                       logger=logger,
                                       log_variables=True, log_expression=True)

    if (results.solver.status == SolverStatus.ok) and (
            results.solver.termination_condition == TerminationCondition.optimal):
        print('---------------------------')
        print('Status: Feasible solution')
        print('\n')
        print('Problem Results:')
        print(results.Problem())
        print('Solver Results:')
        print(results.Solver())
        schedules_dicts = build_solution(model)
        print(model.makespan.pprint())

        return schedules_dicts

    elif results.solver.termination_condition == TerminationCondition.infeasible:
        print('---------------------------')
        print('Status: Infeasible solution (!)')
        return []
    else:
        print('---------------------------')
        print('Status: {}(!)'.format(results.solver.status))
        # Something else is wrong
        print('termination condition:', results.solver.termination_condition)
        return []


def show_schedule(schedules_dicts):
    schedule = DataFrame(schedules_dicts)
    print('\nSchedule by Job')
    print(schedule.sort_values(by=['Job', 'Start']).set_index(['Job', 'Machine']))
    print('\nSchedule by Machine')
    print(schedule.sort_values(by=['Machine', 'Start']).set_index(['Machine', 'Job']))


def visualize(schedules_dicts):
    schedule = DataFrame(schedules_dicts)
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


if __name__ == '__main__':
    # define model
    model = model_jobshop(TASKS)
    # solve model
    results = solve_jobshop(model)
    # Get solution if exists
    schedules_dicts = has_solution(results, model)
    # Show and plot solution
    if schedules_dicts:
        show_schedule(schedules_dicts)
        visualize(schedules_dicts)
