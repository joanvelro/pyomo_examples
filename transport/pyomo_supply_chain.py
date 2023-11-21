from pyomo.environ import *

# Sets
products = ['Product1', 'Product2']  # Set of products
nodes = ['Factory', 'Warehouse1', 'Warehouse2', 'Retailer']  # Set of nodes

# Parameters
demand = {'Retailer': {'Product1': 100, 'Product2': 150}}  # Demand at each retailer for each product
supply_capacity = {'Factory': {'Product1': 200, 'Product2': 300}}  # Supply capacity at each factory for each product
trans_cost = {'Factory': {'Warehouse1': 5, 'Warehouse2': 7},
              'Warehouse1': {'Retailer': 4},
              'Warehouse2': {'Retailer': 6}}  # Transportation cost from each node to another

holding_cost = {'Factory': {'Product1': 2, 'Product2': 3},
                'Warehouse1': {'Product1': 1, 'Product2': 1.5},
                'Warehouse2': {'Product1': 1.5, 'Product2': 2},
                'Retailer': {'Product1': 3, 'Product2': 4}}  # Holding cost at each node for each product

# Model
model = ConcreteModel()

# Decision Variables
model.x = Var(nodes, nodes, products,
              within=NonNegativeReals)  # Quantity shipped from each node to another for each product
model.y = Var(nodes, products,
              within=NonNegativeReals)  # Quantity held at each node for each product

# Objective Function
model.cost = Objective(expr=sum(trans_cost[i][j] * model.x[i, j, p] for i in nodes for j in nodes for p in products) +
                            sum(holding_cost[i][p] * model.y[i, p] for i in nodes for p in products), sense=minimize)

# Constraints
model.demand_constraint = ConstraintList()
for r in demand.keys():
    for p in products:
        model.demand_constraint.add(
            sum(model.x[i, r, p] for i in nodes if i != r) == demand[r][p])  # Demand constraint at each retailer

model.supply_capacity_constraint = ConstraintList()
for f in supply_capacity.keys():
    for p in products:
        model.supply_capacity_constraint.add(sum(model.x[f, j, p] for j in nodes if j != f) == supply_capacity[f][
            p])  # Supply capacity constraint at each factory

model.balance_constraint = ConstraintList()
for j in nodes:
    for p in products:
        model.balance_constraint.add(sum(model.x[i, j, p] for i in nodes if i != j) + model.y[j, p] ==
                                     sum(model.x[j, k, p] for k in nodes if
                                         k != j))  # Flow balance constraint at each node

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model)

# Print the optimal solution
for i in nodes:
    for j in nodes:
        for p in products:
            print(f"Quantity shipped from {i} to {j} for {p}: {model.x[i, j, p].value}")
for j in nodes:
    for p in products:
        print(f"Quantity held at {j} for {p}: {model.y[j, p].value}")
