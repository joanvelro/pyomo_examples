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
n = 5  # coefficients
m = 4  # decision variables
T = 3  # time periods

# cost of raw materials i
c = {1: 15, 2: 20, 3: 25, 4: 55, 5: 12}
# demand in period t of product j
b = {(1, 1): 150,
     (1, 2): 256,
     (1, 3): 280,
     (1, 4): 290,
     (2, 1): 140,
     (2, 2): 286,
     (2, 3): 270,
     (2, 4): 220,
     (3, 1): 110,
     (3, 2): 226,
     (3, 3): 230,
     (3, 4): 270
     }
# a quantity A(i,j) of raw material i is necessary to build a unit of final product j
A = {(i, j): np.random.random() for i in range(1, n+1) for j in range(1, m+1)}

# model
model = pyo.ConcreteModel()

model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

model.i = pyo.RangeSet(1, n)
model.j = pyo.RangeSet(1, m)
model.t = pyo.RangeSet(1, T)

model.x = pyo.Var(model.t, model.i, domain=pyo.PositiveReals) # PositiveReals PositiveIntegers

model.c = pyo.Param(model.i, initialize=c)
model.b = pyo.Param(model.t, model.j, initialize=b)

model.A = pyo.Param(model.i, model.j, initialize=A)


def constraint_demand_rule(model, j, t):
    return sum(model.A[i, j] * model.x[t, i] for i in model.i) >= model.b[t, j]


model.constraint_demand = pyo.Constraint(model.j, model.t, rule=constraint_demand_rule)


def objective_rule(model):
    return sum(model.c[i] * model.x[t, i] for i in model.i for t in model.t)


model.OF = pyo.Objective(rule=objective_rule, sense=pyo.minimize, doc='minimize cost')
print('\n')
print('\n')
print('--- Model Summary ---')
print(model.pprint())

print('\n')
print('\n')
print('--- Model Solver ---')
opt = SolverFactory("glpk") #  glpk

results = opt.solve(model, tee=True)  # tee=True for details

print('\n')
print('\n')
print('--- Model Results ---')
# send results to stdout
#results.write()
# !cat results.yml


# Check the solution
if (results.solver.status == SolverStatus.ok) and (results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())
    print('--- Constraints ---')
    print('Ax:', model.constraint_demand.pprint())
    for index in model.constraint_demand.index_set():
        print('Constraint {}: {}'.format(index, pyo.value(model.constraint_demand[index])))
    print('--- Objective ---')
    print('z:', pyo.value(model.OF) )
    print(model.OF.pprint())

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

