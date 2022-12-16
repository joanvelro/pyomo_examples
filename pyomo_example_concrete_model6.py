#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    This script ...

        max p A X - c x
	st. A x >= b
	st. A x <= d

	b = b1, ..., bm
	c = c1, ..., cn
	A = M_nxm

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import numpy as np

np.random.seed(0)

# data
n = 5  # coefficients (raw materials)
m = 4  # decision variables (final products)
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

production_limit = {(1, 1): 500,
                    (1, 2): 500,
                    (1, 3): 500,
                    (1, 4): 500,
                    (2, 1): 500,
                    (2, 2): 500,
                    (2, 3): 500,
                    (2, 4): 500,
                    (3, 1): 500,
                    (3, 2): 500,
                    (3, 3): 500,
                    (3, 4): 500
                    }
prices = {(1, 1): 21.50,
          (1, 2): 32.56,
          (1, 3): 42.80,
          (1, 4): 72.90,
          (2, 1): 61.40,
          (2, 2): 52.86,
          (2, 3): 32.70,
          (2, 4): 22.20,
          (3, 1): 31.10,
          (3, 2): 24.26,
          (3, 3): 62.30,
          (3, 4): 72.70
          }
# a quantity A(i,j) of raw material i is necessary to build a unit of final product j
A = {(i, j): np.random.random() for i in range(1, n + 1) for j in range(1, m + 1)}

# model
model = pyo.ConcreteModel()

model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

model.i = pyo.RangeSet(1, n)
model.j = pyo.RangeSet(1, m)
model.t = pyo.RangeSet(1, T)

model.x = pyo.Var(model.t, model.i, domain=pyo.PositiveIntegers)  # PositiveReals PositiveIntegers

model.prices = pyo.Param(model.t, model.j, initialize=prices)
model.c = pyo.Param(model.i, initialize=c)
model.b = pyo.Param(model.t, model.j, initialize=b)
model.production_limit = pyo.Param(model.t, model.j, initialize=production_limit)

model.A = pyo.Param(model.i, model.j, initialize=A)


def constraint_demand_rule(model, j, t):
    return sum(model.A[i, j] * model.x[t, i] for i in model.i) >= model.b[t, j]


model.constraint_demand = pyo.Constraint(model.j, model.t, rule=constraint_demand_rule)


def constraint_max_prod_rule(model, j, t):
    return sum(model.A[i, j] * model.x[t, i] for i in model.i) <= model.production_limit[t, j]


model.constraint_production = pyo.Constraint(model.j, model.t, rule=constraint_max_prod_rule)


def production_rule(model, t, j):
    return sum(model.A[i, j] * model.x[t, i] for i in model.i)


model.production = pyo.Expression(model.t, model.j, rule=production_rule)


def cost_rule(model):
    return sum(model.c[i] * model.x[t, i] for i in model.i for t in model.t)


model.total_cost = pyo.Expression(rule=cost_rule)


def income_rule(model):
    return sum(model.production[t, j] * model.prices[t, j] for j in model.j for t in model.t)


# a = [i for i in model.x.items()]

model.total_income = pyo.Expression(rule=income_rule)


def objective_rule(model):
    # total_cost = sum(model.c[i] * model.x[t, i] for i in model.i for t in model.t)
    # total_income = sum(model.production[t, j] * model.prices[t, j] for j in model.j for t in model.t)
    return model.total_income - model.total_cost


model.OF = pyo.Objective(rule=objective_rule, sense=pyo.maximize, doc='maximize profit')
print('\n')
print('\n')
print('--- Model Summary ---')
print(model.pprint())

print('\n')
print('\n')
print('--- Model Solver ---')
opt = SolverFactory("cbc.exe")  # glpk

results = opt.solve(model, tee=True)  # tee=True for details

print('\n')
print('\n')
print('--- Model Results ---')
# send results to stdout
# results.write()
# !cat results.yml


# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())
    print('--- Constraints ---')
    print('Ax:', model.constraint_demand.pprint())
    for index in model.constraint_demand.index_set():
        print('Constraint {}: {}'.format(index, pyo.value(model.constraint_demand[index])))
    print('--- Objective (Profit) ---')
    print('z:', pyo.value(model.OF))
    print(model.OF.pprint())
    print('--- Total Income ---')
    print('I:', model.total_income())
    print(model.total_income.pprint())
    print('--- Total Cost ---')
    print('C:', model.total_cost())
    print(model.total_cost.pprint())

    print('--- Decision variable---')
    print(model.x.pprint())
    print('--- Dual Variables ---')
    print(model.dual.pprint())
    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('(!) Infeasible solution')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
