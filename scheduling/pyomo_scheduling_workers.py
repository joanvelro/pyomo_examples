"""
https://towardsdatascience.com/modeling-and-optimization-of-a-weekly-workforce-with-python-and-pyomo-29484ba065bb

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

import highspy


# Initialize model
model = pyo.ConcreteModel()

param_hours_per_day = 8
param_limit_weekly_hours = 40
param_min_required_workers_per_shift = 3

set_workers = ['w1', 'w2', 'w3', 'w4', 'w5']
set_periods = ['t1', 't2', 't3']
param_days_shifts = {'t1': ['s1', 's2', 's3'],
                     't2': ['s2', 's3'],
                     't3': ['s1', 's3']}

# binary variables representing if a worker is scheduled somewhere
model.var_works = pyo.Var(((worker, day, shift)
                           for worker in set_workers
                           for day in set_periods
                           for shift in param_days_shifts[day]),
                          within=pyo.Binary, initialize=0)

# binary variables representing if a worker is necessary
model.var_workers_needed = pyo.Var(set_workers, within=pyo.Binary, initialize=0)


def obj_rule(model):
    return sum(model.var_workers_needed[worker] for worker in set_workers)


model.obj = pyo.Objective(rule=obj_rule, sense=pyo.minimize)

# Create a set of constraints
model.constraints = pyo.ConstraintList()


for day in set_periods:
    for shift in param_days_shifts[day]:
        model.constraints.add(
            param_min_required_workers_per_shift == sum(model.var_works[worker, day, shift] for worker in set_workers))


# Constraint: no more than 52 hours worked
for worker in set_workers:
    model.constraints.add(
        sum(param_hours_per_day * model.var_works[worker, day, shift]
            for day in set_periods for shift in param_days_shifts[day]) <= param_limit_weekly_hours)


# solver = 'highs'
solver = 'glpk'

if solver == 'highs':
    model.write("scheduling.mps")
    h = highspy.Highs()
    filename = 'scheduling.mps'
    h.readModel(filename)
    h.run()

    solution = h.getSolution()
    basis = h.getBasis()
    info = h.getInfo()
    model_status = h.getModelStatus()
    print('Model status = ', h.modelStatusToString(model_status))
    print()
    print('Optimal objective = ', info.objective_function_value)
    print('Iteration count = ', info.simplex_iteration_count)
    print('Primal solution status = ', h.solutionStatusToString(info.primal_solution_status))
    print('Dual solution status = ', h.solutionStatusToString(info.dual_solution_status))
    print('Basis validity = ', h.basisValidityToString(info.basis_validity))

if solver != 'highs':
    solver = pyo.SolverFactory(solver)
    results = solver.solve(model)
    # results.write()

    # print(model.pprint())

    # Check the solution
    status_ok = results.solver.status == SolverStatus.ok
    feasible = (results.solver.termination_condition == TerminationCondition.feasible)
    optimal = (results.solver.termination_condition == TerminationCondition.optimal)
    infeasible = (results.solver.termination_condition == TerminationCondition.infeasible)
    if status_ok and (feasible or optimal):

        print('\n')
        for ((key), var) in model.var_works.items():
            value = pyo.value(var)
            if value != 0:
                print('var_works({}):{}'.format(key, value))

        print('\n')
        for ((key), var) in model.var_workers_needed.items():
            value = pyo.value(var)
            if value != 0:
                print('var_works_needed({}):{}'.format(key, value))

        print('Model status = ', results.solver.status)
        print('Termination condition = ',results.solver.termination_condition)
        print('Optimal objective: {}'.format(pyo.value(model.obj)))
        # print(model.obj.pprint())

        # Do something when the solution in optimal and feasible
    elif infeasible:
        print('Infeasible solution')
    else:
        print('Status:', results.solver.status)
        # Something else is wrong
        print('termination condition:', results.solver.termination_condition)
