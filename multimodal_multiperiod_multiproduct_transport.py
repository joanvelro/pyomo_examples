import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

from utils import has_solution

# Sets
products = ['Product1', 'Product2']  # Set of products
modes = ['Truck', 'Train', 'Ship']  # Set of transport modes
sources = ['Source1', 'Source2']  # Set of sources
destinations = ['Destination1', 'Destination2']  # Set of destinations
periods = ['Period1', 'Period2']  # Set of periods

# Parameters
supply = {'Source1': {'Product1': 100, 'Product2': 150},
          'Source2': {'Product1': 200, 'Product2': 100}}  # Supply capacity at each source for each product
demand = {'Destination1': {'Product1': 120, 'Product2': 180},
          'Destination2': {'Product1': 80, 'Product2': 120}}  # Demand requirement at each destination for each product
# Cost of shipping from each source to each destination using each mode
shipping_cost = {'Truck': {'Source1': {'Destination1': 5, 'Destination2': 8},
                           'Source2': {'Destination1': 6, 'Destination2': 7}},
                 'Train': {'Source1': {'Destination1': 4, 'Destination2': 6},
                           'Source2': {'Destination1': 5, 'Destination2': 7}},
                 'Ship': {'Source1': {'Destination1': 10, 'Destination2': 12},
                          'Source2': {'Destination1': 9, 'Destination2': 11}}}

production_cost = {'Source1': {'Product1': 10, 'Product2': 12},
                   'Source2': {'Product1': 9, 'Product2': 11}}  # Production cost at each source for each product

# Model
model = pyo.ConcreteModel()

model.set_modes = pyo.Set(initialize=modes)
# Quantity shipped using each mode from each source to each destination for each product in each period
# Decision Variables
model.x = pyo.Var(modes,
                  sources,
                  destinations,
                  products,
                  periods,
                  within=pyo.NonNegativeReals,
                  initialize=0)

# Constraints
model.supply_constraint = pyo.ConstraintList()
for s in sources:
    for p in products:
        for t in periods:
            model.supply_constraint.add(
                sum(model.x[m, s, d, p, t] for m in model.set_modes for d in destinations) <= supply[s][p])

model.demand_constraint = pyo.ConstraintList()
for d in destinations:
    for p in products:
        for t in periods:
            model.demand_constraint.add(sum(model.x[m, s, d, p, t] for m in modes for s in sources) >= demand[d][p])

# Objective Function
model.cost = pyo.Objective(expr=sum(shipping_cost[m][s][d] * model.x[m, s, d, p, t]
                                    for m in modes
                                    for s in sources
                                    for d in destinations
                                    for p in products
                                    for t in periods) +
                                sum(production_cost[s][p] * model.x[m, s, d, p, t]
                                    for m in modes
                                    for s in sources
                                    for d in destinations
                                    for p in products
                                    for t in periods),
                           sense=pyo.minimize)

# Solve the model
solver = SolverFactory('glpk')
results = solver.solve(model, tee=True)

# Print the optimal solution

if has_solution(results, model):
    for ((m, s, d, p, t), var) in model.x.items():
        if pyo.value(var) > 0:
            print(f"Quantity shipped from {s} to {d} for {p} in {t}: {pyo.value(var)}")
