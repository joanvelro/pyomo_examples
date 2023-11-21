"""
In this stochastic model, the additional set model.s represents the set of scenarios. The parameter model.probability
 represents the probability associated with each scenario. The uncertain parameter model.demand_scenario represents the
  demand at each destination for each scenario.

The objective function takes into account the expected cost by summing the product of the scenario probability and the
transportation cost for each scenario. The supply and demand constraints are modified to incorporate the
scenario-specific variables.

Make sure to replace <solver_name> with the name of the solver you have installed (e.g., glpk, cplex, gurobi) before
 running the code.
"""
from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.i = Set()  # Set of sources
model.j = Set()  # Set of destinations
model.s = Set()  # Set of scenarios

# Parameters
model.supply = Param(model.i, within=NonNegativeReals)  # Supply at each source
model.demand = Param(model.j, within=NonNegativeReals)  # Demand at each destination
model.cost = Param(model.i, model.j, within=NonNegativeReals)  # Transportation cost per unit
model.probability = Param(model.s, within=NonNegativeReals)  # Probability of each scenario

# Uncertain Parameters
model.demand_scenario = Param(model.j, model.s, within=NonNegativeReals)  # Demand at each destination for each scenario

# Decision Variables
model.x = Var(model.i, model.j, model.s, within=NonNegativeReals)


# Quantity transported from source i to destination j for each scenario

# Objective Function
def obj_rule(model):
    return sum(
        model.probability[s] * sum(model.cost[i, j] * model.x[i, j, s] for i in model.i for j in model.j) for s in
        model.s)


model.obj = Objective(rule=obj_rule, sense=minimize)


# Constraints
def supply_constraint_rule(model, i, s):
    return sum(model.x[i, j, s] for j in model.j) == model.supply[i]


model.supply_constraint = Constraint(model.i, model.s, rule=supply_constraint_rule)


def demand_constraint_rule(model, j, s):
    return sum(model.x[i, j, s] for i in model.i) == model.demand_scenario[j, s]


model.demand_constraint = Constraint(model.j, model.s, rule=demand_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name> with the name of the solver you want to use
solver.solve(model)

# Print the results
print(f"Optimal cost: {model.obj()}")
for i in model.i:
    for j in model.j:
        for s in model.s:
            print(f"Quantity transported from source {i} to destination {j} in scenario {s}: {model.x[i, j, s].value}")
