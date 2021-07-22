#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    This script ...

        min c x
	st. a x >= b

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

model = pyo.ConcreteModel()

n = 5
model.i = pyo.RangeSet(0, n - 1)
model.x = pyo.Var(model.i, domain=pyo.NonNegativeReals)

c = [3, 4, 5, 6, 7]
a = [7, 3, 2, 1, 9]
b = 100
model.OBJ = pyo.Objective(expr=sum(c[i] * model.x[i] for i in model.i), sense=pyo.minimize)

model.Constraint1 = pyo.Constraint(expr=sum(a[i] * model.x[i] for i in model.i) >= b)

print(model.pprint())

opt = SolverFactory("glpk")

results = opt.solve(model, tee=False)  # tee=True for details

# send results to stdout
results.write()
# !cat results.yml


# Check the solution
if (results.solver.status == SolverStatus.ok) and (results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())
    print('--- Constraint ---')
    print('Ax:', model.Constraint1())
    print('--- Objective ---')
    print('z:', model.OBJ())
    print('--- Decision variable---')
    for i in model.i:

        print('x({}):{}'.format(i, pyo.value(model.x[i])))


    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
