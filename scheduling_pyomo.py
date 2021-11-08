#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
.. module:: scheduling_LS
   :synopsis:  This script define the Scheduling optimization problem using pyomo

.. moduleauthor:: Jose Angel Velasco - (C) Repsol Data Hub - 2021
"""


import pyomo.environ as pyo

model = pyo.ConcreteModel()

model.T = pyo.Set(initialize=[1, 2, 3], doc='Canning plans')

model.demand = pyo.Param(model.T, initialize={1: 2300, 2: 2500, 3: 2000}, doc='demand')
model.price = pyo.Param(model.T, initialize={1: 560, 2: 350, 3: 400}, doc='price')

model.qs = pyo.Var(model.T, bounds=(0, 10000), doc='supply product final')
model.qp = pyo.Var(model.T, bounds=(0, 10000), doc='supply product final')
model.fs = pyo.Var(model.T, bounds=(0, 1), within=pyo.Binary, doc='supply product final')

model.qs[1].fixed = True
model.qs[1].value = 4500


def supply(model, T):
    return (model.qs[T] + model.qp[T]) * model.fs[T] >= model.demand[T]


model.supply = pyo.Constraint(model.T, rule=supply, doc='supply constraint')


def stock(model, T):
    if T == 3:
        return pyo.Constraint.Skip
    else:
        return model.qs[T + 1] == model.qp[T] + model.qs[T] - model.demand[T]


model.stock = pyo.Constraint(model.T, rule=stock, doc='supply constraint')


def dummy(model):
    return sum(model.qs[i] * model.price[i] for i in model.T)


model.OBJ = pyo.Objective(rule=dummy, sense=pyo.minimize)

# Choose Solver
opt = pyo.SolverFactory('ipopt')  # cbc
# Solve Model
results = opt.solve(model, tee=True)

print('qs(T1):', pyo.value(model.qs[1]))
print('qs(T2):', pyo.value(model.qs[2]))
print('qs(T3):', pyo.value(model.qs[3]))

print('fs(T1):', pyo.value(model.fs[1]))
print('fs(T2):', pyo.value(model.fs[2]))
print('fs(T3):', pyo.value(model.fs[3]))

import pandas as pd

df_results=  pd.DataFrame({'qs':[pyo.value(model.qs[i+1]) for i in range(3)],
                           'fs':[pyo.value(model.fs[i+1]) for i in range(3)],
                           'qp':[pyo.value(model.qp[i+1]) for i in range(3)]
})
print('ok')