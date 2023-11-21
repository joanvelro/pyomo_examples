# Import Pyomo and the required components
from pyomo.environ import *

# Create a concrete Pyomo model
model = ConcreteModel()

# Sets
vessels = ['Vessel1', 'Vessel2', 'Vessel3']  # Set of vessels
tanks = ['TankA', 'TankB', 'TankC']  # Set of tanks

# Parameters
vessel_capacity = {'Vessel1': 5000, 'Vessel2': 8000, 'Vessel3': 10000}  # Maximum capacity of each vessel
tank_demand = {'TankA': 3000, 'TankB': 5000, 'TankC': 7000}  # Fuel demand of each tank
transport_cost = {
    ('Vessel1', 'TankA'): 10, ('Vessel1', 'TankB'): 12, ('Vessel1', 'TankC'): 15,
    ('Vessel2', 'TankA'): 8, ('Vessel2', 'TankB'): 10, ('Vessel2', 'TankC'): 14,
    ('Vessel3', 'TankA'): 6, ('Vessel3', 'TankB'): 9, ('Vessel3', 'TankC'): 12
}  # Cost to transport fuel from each vessel to each tank

# Variables
model.x = Var(vessels, tanks, domain=NonNegativeReals)  # Amount of fuel to be transported from each vessel to each tank

# Objective function: Minimize the total transportation cost
model.objective = Objective(
    expr=sum(transport_cost[vessel, tank] * model.x[vessel, tank] for vessel in vessels for tank in tanks),
    sense=minimize)


# Constraints
# 1. Capacity constraint for each vessel
def vessel_capacity_constraint_rule(model, vessel):
    return sum(model.x[vessel, tank] for tank in tanks) <= vessel_capacity[vessel]


model.vessel_capacity_constraint = Constraint(vessels, rule=vessel_capacity_constraint_rule)


# 2. Demand constraint for each tank
def tank_demand_constraint_rule(model, tank):
    return sum(model.x[vessel, tank] for vessel in vessels) >= tank_demand[tank]


model.tank_demand_constraint = Constraint(tanks, rule=tank_demand_constraint_rule)

# Solve the optimization problem
SolverFactory('glpk').solve(model)

# Display the results
for vessel in vessels:
    for tank in tanks:
        print(f"Fuel transported from {vessel} to {tank}: {model.x[vessel, tank]()}")

print(f"Total transportation cost: {model.objective():.2f}")
