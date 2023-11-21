#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    This script ...

        min c x
	st. A x >= b

	b = b1, ..., bm
	c = c1, ..., cn
	A = M_nxm

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import numpy as np


# data
n = 5
m = 4
c = {1: 15, 2: 20, 3: 25, 4: 55, 5: 12}
b = {1: 150, 2: 256, 3: 280, 4: 290}
A = {(i, j): np.random.random() for i in model.i for j in model.j}

# model
model = pyo.ConcreteModel()

model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)
model.i = pyo.RangeSet(1, n)
model.j = pyo.RangeSet(1, m)

model.x = pyo.Var(model.i, domain=pyo.NonNegativeReals)

model.c = pyo.Param(model.i, initialize=c)
model.b = pyo.Param(model.j, initialize=b)

model.A = pyo.Param(model.i, model.j, initialize=A)


def con_rule(model, j):
    return sum(model.A[i, j] * model.x[i] for i in model.i) >= model.b[j]


model.Constraint1 = pyo.Constraint(model.j, rule=con_rule)


def obj_rule(model):
    return sum(model.c[i] * model.x[i] for i in model.i)


model.OBJ = pyo.Objective(rule=obj_rule, sense=pyo.minimize, doc='minimize cost')

print('--- Model Summary ---')
print(model.pprint())

print('--- Model Solver ---')
opt = SolverFactory("glpk")

results = opt.solve(model, tee=False)  # tee=True for details

print('--- Model Results ---')
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
    print('--- Constraints ---')
    print('Ax:', model.Constraint1.pprint())
    for index in model.Constraint1.index_set():
        print('Constraint {}: {}'.format(index, pyo.value(model.Constraint1[index])))
    print('--- Objective ---')
    print('z:', model.OBJ.pprint())
    print('--- Decision variable---')
    print(model.x.pprint())
    print('--- Dual Variables ---')
    print(model.dual.pprint())
    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
