"""
https://towardsdatascience.com/modeling-and-optimization-of-a-weekly-workforce-with-python-and-pyomo-29484ba065bb

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# Initialize model
model = pyo.ConcreteModel()

workers = ['w1', 'w2', 'w3', 'w4', 'w5']
date = ['t1', 't2', 't3']
days_shifts = {'t1': ['s1', 's2', 's3'],
               't2': ['s2', 's3'],
               't3': ['s1', 's3']}

# binary variables representing if a worker is scheduled somewhere
model.works = pyo.Var(((worker, day, shift) for worker in workers for day in date for shift in days_shifts[day]),
                      within=pyo.Binary, initialize=0)

# binary variables representing if a worker is necessary
model.needed = pyo.Var(workers, within=pyo.Binary, initialize=0)


def obj_rule(m):
    c = len(workers)
    return sum(m.needed[worker] for worker in workers)


model.obj = pyo.Objective(rule=obj_rule, sense=pyo.minimize)

# Create a set of constraints
model.constraints = pyo.ConstraintList()

#for day in date:
#    for shift in days_shifts[day]:
#        model.constraints.add(33 == sum(model.works[worker, day, shift] for worker in workers))

# Constraint: no more than 52 hours worked
for worker in workers:
    model.constraints.add(
        52 >= sum(12 * model.works[worker, day, shift] for day in date for shift in days_shifts[day]))

# for worker in workers:
#    model.constraints.add(
#        52 >= sum(shift_len[day, shift] * model.works[worker, day, shift] for day in date for shift in days_shifts[day]))


results = pyo.SolverFactory('glpk').solve(model)
# results.write()

print(model.pprint())

# Check the solution
if (results.solver.status == SolverStatus.ok) and (results.solver.termination_condition == TerminationCondition.optimal):
    print('=== Feasible solution')
    print('Objective Function')
    print(model.obj.pprint())

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
