"""
In this multiperiod multitank fuel oil blending optimization model, the sets model.Periods, model.Components,
and model.Tanks represent the periods, components, and tanks, respectively.

The parameters model.Availability represent the availability of each component in each period, model.TargetSpec
 represent the target specification for each period, model.TankCapacity represent the capacity of each tank, and
 model.InitialInventory represent the initial inventory of each tank
"""
from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.Periods = RangeSet(1, 3)  # Set of periods
model.Components = Set()  # Set of components
model.Tanks = Set()  # Set of tanks

# Parameters
model.Availability = Param(model.Components, model.Periods)  # Availability of components
model.TargetSpec = Param(model.Periods)  # Target specification for each period
model.TankCapacity = Param(model.Tanks)  # Capacity of each tank
model.InitialInventory = Param(model.Tanks)  # Initial inventory of each tank

# Decision Variables
model.Quantity = Var(model.Components, model.Periods, within=NonNegativeReals)  # Quantity of each component
model.Inventory = Var(model.Tanks, model.Periods, within=NonNegativeReals)  # Inventory in each tank


# Objective Function
def obj_rule(model):
    return sum(model.Quantity[c, t] for c in model.Components for t in model.Periods)


model.obj = Objective(rule=obj_rule, sense=minimize)


# Constraints
def availability_constraint_rule(model, c, t):
    return model.Quantity[c, t] <= model.Availability[c, t]


model.availability_constraint = Constraint(model.Components, model.Periods, rule=availability_constraint_rule)


def spec_constraint_rule(model, t):
    return sum(model.Quantity[c, t] for c in model.Components) == model.TargetSpec[t]


model.spec_constraint = Constraint(model.Periods, rule=spec_constraint_rule)


def tank_capacity_constraint_rule(model, k, t):
    return model.Inventory[k, t] <= model.TankCapacity[k]


model.tank_capacity_constraint = Constraint(model.Tanks, model.Periods, rule=tank_capacity_constraint_rule)


def tank_balance_constraint_rule(model, k, t):
    if t == 1:
        return model.Inventory[k, t] == model.InitialInventory[k] + sum(
            model.Quantity[c, t] for c in model.Components if c in model.Tanks[k])
    else:
        return model.Inventory[k, t] == model.Inventory[k, t - 1] + sum(
            model.Quantity[c, t] for c in model.Components if c in model.Tanks[k])


model.tank_balance_constraint = Constraint(model.Tanks, model.Periods, rule=tank_balance_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name> with the name of the solver you want to use
solver.solve(model)

# Print the results
for c in model.Components:
    for t in model.Periods:
        print(f"Period {t}, Component {c}: Quantity = {model.Quantity[c, t].value}")

for k in model.Tanks:
    for t in model.Periods:
        print(f"Period {t}, Tank {k}: Inventory = {model.Inventory[k, t].value}")
