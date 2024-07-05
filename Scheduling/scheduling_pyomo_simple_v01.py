#!/usr/bin/python
# -*- coding: utf-8 -*-
"""  Scheduling Pyomo Simple

    Tank component ---> Tank Blending --> Tank Product Stock --> Demand
"""
import logging

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
from pandas import DataFrame
from pyomo.util.infeasible import log_infeasible_constraints

import logging

logging.getLogger(__name__).addHandler(logging.NullHandler())


def add_stderr_logger(level: int = logging.INFO) -> logging.StreamHandler:
    """
    Helper for quickly adding a StreamHandler to the logger. Useful for
    debugging.
    Returns the handler after adding it.
    """
    # This method needs to be in this __init__.py to get the __name__ correct
    # even if repsolver-multi-period-logistic is vendored within another package.
    logger = logging.getLogger(__name__)
    handler = logging.StreamHandler()
    handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
    logger.addHandler(handler)
    logger.setLevel(level)
    logger.propagate = False
    return handler


add_stderr_logger()
logger = logging.getLogger(__name__)

model = pyo.ConcreteModel()

# ==================
# ::: Sets :::
# ==================
# time periods (days)
T = 8
model.T = pyo.Set(dimen=1, initialize={0, 1, 2, 3, 4, 5, 6, 7, 8})

# ==================
# ::: Parameters :::
# ==================
# maximum capacity
q_b_max = 25000  # blending tank
q_s_max = 25000  # stock tank
# maximum capacity
q_b_min = 500  # blending tank
q_s_min = 500  # stock tank
# initial stocks
q_s_0 = 5000
q_b_0 = 1000
# Demand to supply
demand = [0, 0, 0, 15000, 0, 0, 25000, 0, 10000]
purchase_cost = 100
stock_cost = 100
# minimum quantity to purchase
x_min = 250
x_max = 6000
# Flow rate
flow_rate = 5000

# Variables
# Binary Variables
model.y_bs = pyo.Var(model.T, domain=pyo.Binary)
model.y = pyo.Var(model.T, domain=pyo.Binary)
# Tanks volume variables
# Product stock tank quantity
model.qs = pyo.Var(model.T, domain=pyo.NonNegativeReals, bounds=(q_s_min, q_s_max))
# Blending tanks quantity
model.qb = pyo.Var(model.T, domain=pyo.NonNegativeReals, bounds=(0, q_b_max))
# Pipe flow
model.f_bs = pyo.Var(model.T, domain=pyo.NonNegativeReals, bounds=(0, flow_rate))
# quantity to blend
model.x = pyo.Var(model.T, domain=pyo.NonNegativeReals, bounds=(0, x_max))


# ==================
# ::: Constraints :::
# ==================

def min_purchase(model, t):
    return model.x[t] >= x_min * model.y[t]


def balance_stock_tank(model, t):
    if t == 0:
        return model.qs[t] == q_s_0
    elif t <= T:
        return model.qs[t] == model.qs[t-1] - demand[t-1] + model.x[t]
    else:
        return pyo.Constraint.Skip


def balance_blending_tank(model, t):
    if t == 0:
        model.qb[t] = q_b_0
    if 0 < t < T:
        return model.qb[t + 1] == model.qb[t] + model.x[t] - model.f_bs[t]
    else:
        return pyo.Constraint.Skip



def meet_demand(model, t):
    return model.qs[t] >= demand[t]

def flow_from_blending(model, t):
    return model.f_bs[t] == flow_rate * model.y_bs[t]




# Constraints
model.min_purchase = pyo.Constraint(model.T, rule=min_purchase)
model.balance_stock_tank = pyo.Constraint(model.T, rule=balance_stock_tank)
# model.balance_blending_tank = pyo.Constraint(model.T, rule=balance_blending_tank)
#model.supply_demand = pyo.Constraint(rule=sum(model.qs[t] for t in model.T) >= sum(demand[t] for t in model.T))
model.meet_demand = pyo.Constraint(model.T, rule=meet_demand)
# model.flow_from_blending = pyo.Constraint(model.T, rule=flow_from_blending)

# Objective
model.OBJ = pyo.Objective(expr=sum(purchase_cost * model.x[t] for t in model.T),
                          sense=pyo.minimize)

# Print model
# print(model.pprint())

opt = SolverFactory("cbc")

results = opt.solve(model, tee=True)  # tee=True for details


# send results to stdout
# results.write()
# !cat results.yml


def has_solution():
    status = results.solver.status
    condition = results.solver.termination_condition

    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    status = results.solver.status
    condition = results.solver.termination_condition

    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    if not solution:
        print(results)
        print(f'Something is wrong -> status {status} and condition {condition}')
        if (condition == pyo.TerminationCondition.infeasible) and (logger.getEffectiveLevel() <= logging.DEBUG):
            log_infeasible_constraints(model, logger=logger, log_expression=True, log_variables=True)
    return solution


if has_solution():
    # Check the solution
    if (results.solver.status == SolverStatus.ok) and (
            results.solver.termination_condition == TerminationCondition.optimal):
        print('\n')
        print('-------------------------')
        print('Status: Optimal Solution')
        print('-------------------------')
        # print('--- Problem Results ---')
        # print(results.Problem())
        # print('--- Solver Results ---')
        # print(results.Solver())
        print('Objective: {}'.format(pyo.value(model.OBJ) / 1000000))
        print('Time: {}'.format(results.Solver()['Time']))
        # print('--- Objective ---')
        # print('z:', model.OBJ.values())
        # print('--- Decision variable---')
        # for t in range(0, T + 1):
        #    print('qs({}):{}'.format(t, pyo.value(model.qs[t])))
        # for t in range(0, T + 1):
        #    print('qb({}):{}'.format(t, pyo.value(model.qb[t])))
        # for t in range(0, T + 1):
        #    print('f_ds({}):{}'.format(t, pyo.value(model.f_ds[t])))

        df_results = DataFrame({
            #'QBlending': [pyo.value(model.qb[i]) for i in range(0, T + 1)],
            #'QComponent': [pyo.value(model.qc[i]) for i in range(0, T + 1)],
            'QStock': [pyo.value(model.qs[i]) for i in model.T],
            # 'CompToBlending': [pyo.value(model.f_cb[i]) for i in range(0, T + 1)],
            #'BlendToStock': [pyo.value(model.f_bs[i]) for i in range(0, T + 1)],
            # 'ComponentToBlending': [pyo.value(model.y_cb[i]) for i in range(0, T + 1)],
            # 'BlendingToStock': [pyo.value(model.y_bs[i]) for i in range(0, T + 1)],
            'QuantityPurchased': [pyo.value(model.x[i]) for i in range(0, T + 1)],
            'Demand': demand})

        print(df_results[['QStock']])
        print(df_results[['QuantityPurchased', 'Demand']])

    elif (results.solver.status == SolverStatus.ok) and (
            results.solver.termination_condition == TerminationCondition.feasible):
        print('\n')
        print('-------------------------')
        print('Status: Feasible Solution')
        print('-------------------------')
        # print('--- Problem Results ---')
        # print(results.Problem())
        # print('--- Solver Results ---')
        # print(results.Solver())
        print('Objective: {}'.format(pyo.value(model.OBJ) / 1000000))
        print('Time: {}'.format(results.Solver()['Time']))
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('\n')
    print('----------------------')
    print('Status: Infeasible')
    print('----------------------')
    print('\n')
else:
    print('\n')
    print('----------------------')
    print('Status: {}'.format(results.solver.status))
    print('Termination condition:', results.solver.termination_condition)
    print('---------------------')
    print('\n')
    # Something else is wrong
