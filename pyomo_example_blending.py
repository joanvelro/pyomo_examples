"""
In this multiperiod blending optimization model, the sets model.Periods and model.Materials represent the periods and raw materials, respectively.

The parameters model.RawCost represent the cost of each raw material, and model.Availability represent the availability of each raw material in each period.

The parameter model.TargetSpec represents the target specification for the product.

The decision variable model.Quantity represents the quantity of each raw material used in each period.

The objective function aims to minimize the total cost, calculated as the sum of the cost of each raw material used.

The constraints ensure that the availability of each raw material is not exceeded and that the sum of the quantities of raw materials meets the target specification for each period.

Make sure to replace <solver_name> with the name of the solver you have installed (e.g., glpk, cplex, gurobi) before running the code.

Note: This is a simplified example, and there are various other factors and constraints that can be considered in a blending optimization problem, such as quality constraints, storage limitations, and production capacity.

"""
from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.Periods = RangeSet(1, 3)  # Set of periods
model.Materials = Set()  # Set of raw materials

# Parameters
model.RawCost = Param(model.Materials)  # Cost of raw materials
model.Availability = Param(model.Materials, model.Periods)  # Availability of raw materials
model.TargetSpec = Param()  # Target specification for the product

# Decision Variables
model.Quantity = Var(model.Materials, model.Periods, within=NonNegativeReals)  # Quantity of each raw material

# Objective Function
def obj_rule(model):
    return sum(model.RawCost[m] * model.Quantity[m, t] for m in model.Materials for t in model.Periods)
model.obj = Objective(rule=obj_rule, sense=minimize)

# Constraints
def availability_constraint_rule(model, m, t):
    return model.Quantity[m, t] <= model.Availability[m, t]
model.availability_constraint = Constraint(model.Materials, model.Periods, rule=availability_constraint_rule)

def spec_constraint_rule(model, t):
    return sum(model.Quantity[m, t] for m in model.Materials) == model.TargetSpec
model.spec_constraint = Constraint(model.Periods, rule=spec_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name> with the name of the solver you want to use
solver.solve(model)

# Print the results
for m in model.Materials:
    for t in model.Periods:
        print(f"Period {t}, Material {m}: Quantity = {model.Quantity[m, t].value}")