import pyomo.environ as pyo
from pandas import DataFrame
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

model = pyo.ConcreteModel()

# Set
# Time periods
model.set_periods = pyo.Set(initialize=[0, 1, 2, 3], doc='time periods')

# Parameters
# Demand
model.param_demand = pyo.Param(model.set_periods, initialize={1: 2300, 2: 2500, 3: 2000}, doc='param demand')
# Price
model.param_purchase_price = pyo.Param(model.set_periods, initialize={1: 560, 2: 350, 3: 400}, doc='param price')
model.param_sell_price = pyo.Param(model.set_periods, initialize={1: 1560, 2: 1350, 3: 1400}, doc='param price')

# Variables
# Storage tank
model.var_q_stock = pyo.Var(model.set_periods, bounds=(0, 10000), within=pyo.NonNegativeIntegers,
                            doc='supply product final', initialize=100)
# Input truck
model.var_q_input = pyo.Var(model.set_periods, bounds=(0, 10000), within=pyo.NonNegativeIntegers, doc='supply product final')


# Fix stock
model.var_q_stock[0].fixed = True
model.var_q_stock[0].value = 500
model.var_q_input[0].fixed = True
model.var_q_input[0].value = 0

def supply_exp_rule(model, t):
    if t > 0:
        return (model.var_q_stock[t] - model.var_q_stock[t-1]) + model.var_q_input[t]
    else:
        return pyo.Expression.Skip


model.exp_supply = pyo.Expression(model.set_periods, rule=supply_exp_rule, doc='supply expression')


def supply_rule(model, t):
    if t > 0:
        return model.exp_supply[t] >= model.param_demand[t]
    else:
        return pyo.Constraint.Skip


model.constraint_supply = pyo.Constraint(model.set_periods, rule=supply_rule, doc='supply constraint')


# Stock

def stock(model, t):
    if t > 0:
        return model.var_q_stock[t] == model.var_q_stock[t-1] + model.var_q_input[t]
    else:
        return pyo.Constraint.Skip


model.constraint_stock = pyo.Constraint(model.set_periods, rule=stock, doc='supply constraint')


def objective(model):
    total_cost = sum(model.var_q_input[t] * model.param_purchase_price[t] for t in model.set_periods if t != 0)
    total_revenue = sum(model.exp_supply[t] * model.param_sell_price[t] for t in model.set_periods if t != 0)
    return total_revenue - total_cost


model.objective = pyo.Objective(rule=objective, sense=pyo.minimize)

# Choose Solver
opt = pyo.SolverFactory('scip')  # cbc
# Solve Model
results = opt.solve(model, tee=True)

status_ok = results.solver.status == SolverStatus.ok
feasible = results.solver.termination_condition == TerminationCondition.feasible
infeasible = results.solver.termination_condition == TerminationCondition.infeasible
optimal = results.solver.termination_condition == TerminationCondition.optimal
if status_ok and (feasible or optimal):
    print('\n')
    if feasible:
        print('FEASIBLE Solution founded!')
    if optimal:
        print('OPTIMAL Solution founded!')
    print('\n')
    for ((key), var) in model.var_q_stock.items():
        value_ = pyo.value(var)
        print('var_q_stock({}):{}'.format(key, value_))
    print('\n')
    for ((key), var) in model.var_q_input.items():
        value_ = pyo.value(var)
        print('var_q_input({}):{}'.format(key, value_))
    print('\n')
    for ((key), var) in model.exp_supply.items():
        value_ = pyo.value(var)
        print('exp_supply({}):{}'.format(key, value_))
    print('\n')
    print('Objective:{} M euros'.format(pyo.value(model.objective)/1e6))




print('ok')
