"""
In this long-term scheduling model, the set model.T represents the months in the planning horizon, and the set model.P
represents the products produced by the food-processing plant.

The parameters model.Demand, model.ProductionCapacity, model.InventoryCost, and model.InitialInventory represent the
monthly demand for each product, the production capacity for each month, the cost of carrying inventory for each
product, and the initial inventory levels for each product, respectively.

The decision variables model.Production and model.Inventory represent the monthly production quantities and inventory
 levels for each product.

The objective function aims to minimize the end-of-horizon inventory cost, which is calculated as the sum of the
 inventory cost multiplied by the inventory level for each product at the end of the planning horizon.

The constraints ensure that the production quantities do not exceed the monthly production capacities and maintain the
 inventory balance by considering the initial inventory, production, and demand for each month and product.

Make sure to replace <solver_name> with the name of the solver you
"""
from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.T = RangeSet(1, 12)  # Set of months in the planning horizon
model.P = Set()  # Set of products

# Parameters
model.Demand = Param(model.T, model.P)  # Monthly demand for each product
model.ProductionCapacity = Param(model.T)  # Monthly production capacity
model.InventoryCost = Param(model.P)  # Cost of carrying inventory for each product
model.InitialInventory = Param(model.P)  # Initial inventory for each product

# Decision Variables
model.Production = Var(model.T, model.P, within=NonNegativeReals)  # Monthly production quantity for each product
model.Inventory = Var(model.T, model.P, within=NonNegativeReals)  # Monthly inventory level for each product


# Objective Function
def obj_rule(model):
    return sum(
        model.InventoryCost[p] * model.Inventory[12, p] for p in model.P)  # Minimize end-of-horizon inventory cost


model.obj = Objective(rule=obj_rule, sense=minimize)


# Constraints
def production_capacity_constraint_rule(model, t):
    return sum(model.Production[t, p] for p in model.P) <= model.ProductionCapacity[t]


model.production_capacity_constraint = Constraint(model.T, rule=production_capacity_constraint_rule)


def inventory_balance_constraint_rule(model, t, p):
    if t == 1:
        return model.Inventory[t, p] == model.InitialInventory[p] + model.Production[t, p] - model.Demand[t, p]
    else:
        return model.Inventory[t, p] == model.Inventory[t - 1, p] + model.Production[t, p] - model.Demand[t, p]


model.inventory_balance_constraint = Constraint(model.T, model.P, rule=inventory_balance_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name> with the name of the solver you want to use
solver.solve(model)

# Print the results
for t in model.T:
    print(f"Month {t}:")
    for p in model.P:
        print(f"   Production of Product {p}: {model.Production[t, p].value}")
        print(f"   Inventory of Product {p}: {model.Inventory[t, p].value}")
