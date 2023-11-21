#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
.. module:: scheduling_LS
   :synopsis:  This script defines the Scheduling optimization problem using pyomo

.. moduleauthor:: Jose Angel Velasco - (C) Hybrid Intelligence Capgemini Engineering - 2022
"""

from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import pyomo.environ as pyo
import pandas as pd
from pyomo.opt import SolverStatus
import numpy as np
model = pyo.ConcreteModel()

# Set
# Time steps
T = 24*1
model.T = pyo.Set(initialize=list(np.arange(0, T +1)), doc='Timeperiods')

# Parameters
# Demand
model.demand = pyo.Param(model.T, initialize={1: 2300, 2: 2500, 3: 2000}, doc='demand')
# Price
model.price = pyo.Param(model.T, initialize={1: 560, 2: 350, 3: 400}, doc='price')

# Variables
# Storage tank
model.Q_stock = pyo.Var(model.T, bounds=(0, 10000))
model.Q_input = pyo.Var(model.T, bounds=(0, 10000))
# Binary decisions
model.b_input = pyo.Var(model.T, bounds=(0, 1), within=pyo.Binary)
model.b_past = pyo.Var(model.T, bounds=(0, 1), within=pyo.Binary)

# Product to be produced
model.q_product_A = pyo.Var(model.T)

F = 100  # kg/min

Q_demand = 1500
Q_input = 1500
Q_stock0 = 100

# Fix stock
model.Q_stock[1].fixed = True
model.Q_stock[1].value = Q_stock0
model.Q_input[1].fixed = True
model.Q_input[1].value = Q_input

model.cons1 = pyo.Constraint(expr=sum(model.b_past[T] * F for T in model.T) >= Q_demand)

model.cons3 = pyo.Constraint(expr=sum(model.b_input[T] * F for T in model.T) >= Q_input)


def stock(model, t):
    if t == T:
        return pyo.Constraint.Skip
    else:
        return model.Q_stock[t + 1] == model.Q_stock[t] + \
               model.b_input[t] * F - \
               model.b_past[t] * F


model.stock = pyo.Constraint(model.T, rule=stock)


def input(model, t):
    if t == T:
        return pyo.Constraint.Skip
    else:
        return model.Q_input[t + 1] == model.Q_input[t] - model.b_input[t] * F


model.past = pyo.Constraint(model.T, rule=input)


# def supply(model, T):
#    return model.q_product_A[T] >= model.demand[T]


# model.supply = pyo.Constraint(model.T, rule=supply)


def objective(model):
    return 1


model.OBJ = pyo.Objective(rule=objective, sense=pyo.minimize)

# Choose Solver
opt = pyo.SolverFactory('cbc')  # cbc
# Solve Model
results = opt.solve(model, tee=True)

# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print('=== Feasible solution')
    print('Objective Function')
    print(model.pprint())

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)

if 1 == 0:
    print('qs(T1):', pyo.value(model.qs[1]))
    print('qs(T2):', pyo.value(model.qs[2]))
    print('qs(T3):', pyo.value(model.qs[3]))

    print('fs(T1):', pyo.value(model.fs[1]))
    print('fs(T2):', pyo.value(model.fs[2]))
    print('fs(T3):', pyo.value(model.fs[3]))

    df_results = pd.DataFrame({'qs': [pyo.value(model.qs[i + 1]) for i in range(3)],
                               'fs': [pyo.value(model.fs[i + 1]) for i in range(3)],
                               'qp': [pyo.value(model.qp[i + 1]) for i in range(3)]
                               })
    print('ok')
