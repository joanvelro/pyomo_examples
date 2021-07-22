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


model.x = pyo.Var([1, 2, 3, 4, 5], domain=pyo.NonNegativeReals)

c = [30, 40, 50, 60, 70]
a = [0.05, 0.18, 0.25, 0.28, 0.33]
b = 100
# model.OBJ = pyo.Objective(expr=2 * model.x[1] + 3 * model.x[2])
model.OBJ = pyo.Objective(expr=sum(c[i] * model.x[i+1] for i in range(5)), sense=pyo.minimize)

# model.Constraint1 = pyo.Constraint(expr=3 * model.x[1] + 4 * model.x[2] >= 1)
model.Constraint1 = pyo.Constraint(expr=sum(a[i] * model.x[i+1] for i in range(5)) >= b)

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
    print('x(1):', pyo.value(model.x[1]))
    print('x(2):', pyo.value(model.x[2]))
    print('x(3):', pyo.value(model.x[3]))
    print('x(4):', pyo.value(model.x[4]))
    print('x(5):', pyo.value(model.x[5]))


    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
