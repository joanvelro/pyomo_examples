from pyomo.environ import *
from pyomo.gdp import *
from pyomo.util.infeasible import log_infeasible_constraints

import logging

logging.getLogger('pyomo.core').setLevel(logging.ERROR)

TASKS = {
    ('Paper_1', 'Blue'): {'dur': 45, 'prec': None},
    ('Paper_1', 'Yellow'): {'dur': 10, 'prec': ('Paper_1', 'Blue')},
    ('Paper_2', 'Blue'): {'dur': 20, 'prec': ('Paper_2', 'Green')},
    ('Paper_2', 'Green'): {'dur': 10, 'prec': None},
    ('Paper_2', 'Yellow'): {'dur': 34, 'prec': ('Paper_2', 'Blue')},
    ('Paper_3', 'Blue'): {'dur': 12, 'prec': ('Paper_3', 'Yellow')},
    ('Paper_3', 'Green'): {'dur': 17, 'prec': ('Paper_3', 'Blue')},
    ('Paper_3', 'Yellow'): {'dur': 28, 'prec': None},
}

SUB_TASKS = [
    {
        ('Paper_1', 'Blue'): {'dur': TASKS[('Paper_1', 'Blue')]['dur'], 'prec': None},
        ('Paper_2', 'Green'): {'dur': TASKS[('Paper_2', 'Green')]['dur'], 'prec': None},
        ('Paper_3', 'Yellow'): {'dur': TASKS[('Paper_3', 'Yellow')]['dur'], 'prec': None}
    },
    {
        ('Paper_1', 'Blue'): {'dur': TASKS[('Paper_1', 'Blue')]['dur'],
                              'prec': None},
        ('Paper_2', 'Green'): {'dur': TASKS[('Paper_2', 'Green')]['dur'],
                               'prec': None},
        ('Paper_3', 'Yellow'): {'dur': TASKS[('Paper_3', 'Yellow')]['dur'],
                                'prec': None},
        ('Paper_1', 'Yellow'): {'dur': TASKS[('Paper_1', 'Yellow')]['dur'],
                                'prec': ('Paper_1', 'Blue')},
        ('Paper_2', 'Blue'): {'dur': TASKS[('Paper_2', 'Blue')]['dur'],
                              'prec': ('Paper_2', 'Green')},
        ('Paper_3', 'Blue'): {'dur': TASKS[('Paper_3', 'Blue')]['dur'],
                              'prec': ('Paper_3', 'Yellow')},
    },
]


def jobshop_model(TASKS):
    model = ConcreteModel()

    # tasks is a two dimensional set of (j,m) constructed from the dictionary keys
    model.TASKS = Set(initialize=TASKS.keys(), dimen=2)

    # the set of jobs is constructed from a python set
    model.JOBS = Set(initialize=list(set([j for (j, m) in model.TASKS])))

    # set of machines is constructed from a python set
    model.MACHINES = Set(initialize=list(set([m for (j, m) in model.TASKS])))

    # the order of tasks is constructed as a cross-product of tasks and filtering
    model.TASKORDER = Set(initialize=model.TASKS * model.TASKS, dimen=4,
                          filter=lambda model, j, m, k, n: (k, n) ==
                                                           TASKS[(j, m)][
                                                               'prec'])

    # the set of disjunctions is cross-product of jobs, jobs, and machines
    model.DISJUNCTIONS = Set(
        initialize=model.JOBS * model.JOBS * model.MACHINES, dimen=3,
        filter=lambda model, j, k, m: j < k and (j, m) in model.TASKS and (
            k, m) in model.TASKS)

    # load duration data into a model parameter for later access
    model.dur = Param(model.TASKS,
                      initialize=lambda model, j, m: TASKS[(j, m)]['dur'])

    # establish an upper bound on makespan
    ub = 90

    # create decision variables
    model.makespan = Var(bounds=(0, ub))
    model.start = Var(model.TASKS, bounds=(0, ub))

    model.objective = Objective(expr=model.makespan, sense=minimize)

    model.finish = Constraint(model.TASKS, rule=lambda model, j, m:
    model.start[j, m] + model.dur[j, m] <= model.makespan)

    model.preceding = Constraint(model.TASKORDER,
                                 rule=lambda model, j, m, k, n:
                                 model.start[k, n] + model.dur[k, n] <=
                                 model.start[j, m])

    model.disjunctions = Disjunction(model.DISJUNCTIONS,
                                     rule=lambda model, j, k, m:
                                     [model.start[j, m] + model.dur[j, m] <=
                                      model.start[k, m],
                                      model.start[k, m] + model.dur[k, m] <=
                                      model.start[j, m]])

    TransformationFactory('gdp.hull').apply_to(model)
    return model


def solver(model, TASKS):
    results = SolverFactory('cbc').solve(model(TASKS))
    return results


if solver(jobshop_model, TASKS).solver.termination_condition == TerminationCondition.infeasible:
    if solver(jobshop_model, SUB_TASKS[0]).solver.termination_condition == TerminationCondition.infeasible:
        print("No es factible en el primer paso")
    else:
        if solver(jobshop_model, SUB_TASKS[1]).solver.termination_condition == TerminationCondition.infeasible:
            print("No es factible en el segundo paso")
        else:
            print("No es factible en el tercer paso")
else:
    print("El problema es factible")

logging.basicConfig(filename='example.log', encoding='utf-8', level=logging.INFO)

m = ConcreteModel()
m.LE = set([1, 2, 3])
m.x = Var(m.LE, initialize=5)


def obj_rule(m):
    return sum(m.x[i] * 1 for i in m.LE)


m.z = Objective(rule=obj_rule, sense=minimize)
m.cons1 = ConstraintList()

for i in m.LE:
    m.cons1.add(m.x[i] >= 3)
    m.cons1.add(m.x[i] <= 2)

solver = SolverFactory('cbc')
solution = solver.solve(m, tee=False)
log_infeasible_constraints(m, log_expression=True, log_variables=True)
logging.basicConfig(filename='example.log', encoding='utf-8', level=logging.INFO)
