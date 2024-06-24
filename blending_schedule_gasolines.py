"""
    This script defines a simplified blending problem cast as a linear programming problem with multiple tanks and
    different time periods using pyomo

"""
import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import pandas as pd
import numpy as np

np.random.seed(0)

# Define sets
PRODUCTS = ['Gasoline', 'Kerosine', 'Fuel Oil', 'Residual']
FEEDS = ['Crude 1', 'Crude 2']
TIME = ['t1', 't2', 't3', 't4']
TANKS = ['Tk1', 'Tk2', 'Tk3', 'Tk4']

# raw materials random demand (x) (3D)
demand = {(o, t, f): round(100 * np.random.random()) for o in TANKS for t in TIME for f in FEEDS}

# Random maximum capacity of production (y) (3D)
maximum_capacity = {(o, t, p): round(100000 * np.random.random()) for o in TANKS for t in TIME for p in PRODUCTS}

# product selling prices and supply costs (2D)
df_products = pd.DataFrame(index=PRODUCTS)
df_products['Price'] = [82, 88, 52, 60]
df_products['supply_cost'] = [8, 9, 5, 6]

# materials costs (2D)
df_crudes = pd.DataFrame(index=FEEDS)
df_crudes['Processing Cost'] = [12, 13]
df_crudes['Feed Costs'] = [48, 30]

# Product conversion from materials (2D)
product_yield = {('Gasoline', 'Crude 1'): 0.8,
                 ('Kerosine', 'Crude 1'): 0.05,
                 ('Fuel Oil', 'Crude 1'): 0.10,
                 ('Residual', 'Crude 1'): 0.05,
                 ('Gasoline', 'Crude 2'): 0.44,
                 ('Kerosine', 'Crude 2'): 0.10,
                 ('Fuel Oil', 'Crude 2'): 0.36,
                 ('Residual', 'Crude 2'): 0.10}

# model formulation ---------------------------------------------------------------------------------------------------
model = pyo.ConcreteModel()

# variables -------------------------------------------------------------------------------------------------------
model.var_inflow = pyo.Var(((tank, feed, time) for tank in TANKS for feed in FEEDS for time in TIME),
                           domain=pyo.NonNegativeReals)
model.var_supply = pyo.Var(((tank, product, time) for tank in TANKS for product in PRODUCTS for time in TIME),
                           domain=pyo.NonNegativeReals)

# objective  -------------------------------------------------------------------------------------------------------
# (maximize profit = income- total costs)
income = sum(df_products.loc[p, 'Price'] * model.var_supply[o, p, t]
             for o in TANKS
             for p in PRODUCTS
             for t in TIME)
raw_materials_cost = sum(
    (df_crudes.loc[f, 'Feed Costs'] + df_crudes.loc[f, 'Processing Cost']) * model.var_inflow[o, f, t]
    for o in TANKS
    for f in FEEDS
    for t in TIME)
supply_cost = sum(df_products.loc[p, 'supply_cost'] * model.var_supply[o, p, t]
                  for o in TANKS
                  for p in PRODUCTS for t in TIME)
profit = income - raw_materials_cost - supply_cost
model.objective = pyo.Objective(expr=profit, sense=pyo.maximize)

#  constraints -------------------------------------------------------------------------------------------------------
model.constraints = pyo.ConstraintList()
for o in TANKS:
    for p in PRODUCTS:
        for t in TIME:
            model.constraints.add(model.var_supply[o, p, t] <= maximum_capacity[o, t, p])
            model.constraints.add(
                model.var_supply[o, p, t] == sum(model.var_inflow[o, f, t] * product_yield[(p, f)] for f in FEEDS))
for o in TANKS:
    for f in FEEDS:
        for t in TIME:
            model.constraints.add(model.var_inflow[o, f, t] >= demand[o, t, f])


# ---------------------------------------------------------------------------------------------------------------------
def pyomo_postprocess(model):
    model.var_inflow.display()
    model.var_supply.display()
    print('profit: {} M$'.format(round(profit() / 1000000, 2)))
    print('income: {} M$'.format(round(income() / 1000000), 2))
    print('raw_materials_cost:{} M$'.format(round(raw_materials_cost() / 1000000), 2))


# model.pprint() # to show the model

# choose the solver appropriate for the problem
opt = SolverFactory("scip")

results = opt.solve(model)  # tee=True for details

# sends results to stdout
# results.write()

# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print('Feasible solution')
    pyomo_postprocess(model)

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
    pyomo_postprocess(model)
