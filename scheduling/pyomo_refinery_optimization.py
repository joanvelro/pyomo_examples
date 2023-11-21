from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.T = RangeSet(1, 7)  # Set of time periods (days, weeks, etc.)
model.P = Set()  # Set of products
model.R = Set()  # Set of refinery units

# Parameters
model.Demand = Param(model.T, model.P)  # Demand for each product in each time period
model.Cost = Param(model.R)  # Operating cost for each refinery unit
model.Capacity = Param(model.R)  # Capacity of each refinery unit
model.Yield = Param(model.R, model.P)  # Yield of each refinery unit for each product
model.InventoryInitial = Param(model.P)  # Initial inventory level for each product
model.InventoryMin = Param(model.P)  # Minimum inventory level for each product
model.InventoryMax = Param(model.P)  # Maximum inventory level for each product

# Decision Variables
model.Production = Var(model.T, model.P, within=NonNegativeReals)  # Production level for each product in each time period
model.Inventory = Var(model.T, model.P, within=NonNegativeReals)  # Inventory level for each product in each time period
model.ResourceAllocation = Var(model.R, model.T, within=NonNegativeReals)  # Resource allocation for each refinery unit in each time period

# Objective Function
def obj_rule(model):
    return sum(model.Production[t, p] * model.Yield[r, p] * model.ResourceAllocation[r, t] - model.Cost[r] * model.ResourceAllocation[r, t]
               for t in model.T for p in model.P for r in model.R)
model.obj = Objective(rule=obj_rule, sense=maximize)

# Constraints
def production_constraint_rule(model, t, p):
    return model.Production[t, p] == sum(model.Yield[r, p] * model.ResourceAllocation[r, t] for r in model.R)
model.production_constraint = Constraint(model.T, model.P, rule=production_constraint_rule)

def capacity_constraint_rule(model, r, t):
    return model.ResourceAllocation[r, t] <= model.Capacity[r]
model.capacity_constraint = Constraint(model.R, model.T, rule=capacity_constraint_rule)

def demand_constraint_rule(model, t, p):
    return model.Production[t, p] <= model.Demand[t, p]
model.demand_constraint = Constraint(model.T, model.P, rule=demand_constraint_rule)

def inventory_balance_constraint_rule(model, t, p):
    if t == 1:
        return model.Inventory[t, p] == model.InventoryInitial[p] + model.Production[t, p] - model.Demand[t, p]
    else:
        return model.Inventory[t, p] == model.Inventory[t-1, p] + model.Production[t, p] - model.Demand[t, p]
model.inventory_balance_constraint = Constraint(model.T, model.P, rule=inventory_balance_constraint_rule)

def inventory_limit_constraint_rule(model, t, p):
    return (model.Inventory[t, p] >= model.InventoryMin[p]) and (model.Inventory[t, p] <= model.InventoryMax[p])
model.inventory_limit_constraint = Constraint(model.T, model.P, rule=inventory_limit_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name>