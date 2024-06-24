from pyomo.environ import *

# Define the model
model = ConcreteModel()

# Sets and Parameters
num_assets = 3  # Number of assets (e.g., stocks, derivatives)
num_scenarios = 5  # Number of scenarios

# Indices
model.assets = RangeSet(1, num_assets)
model.scenarios = RangeSet(1, num_scenarios)

# Parameters
expected_return = {1: 0.08,
                   2: 0.1,
                   3: 0.12}  # Example expected returns for assets
variance = {1: 0.1,
            2: 0.15,
            3: 0.2}  # Example variance for assets

investment_bounds = {1: (0, 0.4),
                     2: (0, 0.5),
                     3: (0, 0.3)}  # Example investment bounds

# Stochastic parameters (example)
scenario_probabilities = {1: 0.2,
                          2: 0.2,
                          3: 0.2,
                          4: 0.2,
                          5: 0.2}  # Example scenario probabilities
limit_risk = 0.2

# (scenario, asset)
scenario_returns = {
    (1, 1): 0.06,
    (1, 2): 0.08,
    (1, 3): 0.1,
    (2, 1): 0.07,
    (2, 2): 0.09,
    (2, 3): 0.11,
    (3, 1): 0.08,
    (3, 2): 0.1,
    (3, 3): 0.12,
    (4, 1): 0.09,
    (4, 2): 0.11,
    (4, 3): 0.13,
    (5, 1): 0.1,
    (5, 2): 0.12,
    (5, 3): 0.14
}  # Example scenario returns

# Variables
model.x = Var(model.assets, within=NonNegativeReals)

# Objective function
model.expected_value = Objective(
    expr=sum(expected_return[i] * model.x[i] for i in model.assets),
    sense=maximize
)

# Constraints
def budget_constraint_rule(model):
    return sum(model.x[i] for i in model.assets) == 1.0

model.budget_constraint = Constraint(rule=budget_constraint_rule)

def risk_constraint_rule(model, s):
    return sum(scenario_probabilities[s] *
               (sum(scenario_returns[s, i] * model.x[i] for i in model.assets) - expected_return[i]) ** 2
               for i in model.assets) <= limit_risk  # Example risk constraint

model.risk_constraint = Constraint(model.scenarios, rule=risk_constraint_rule)

# Bounds constraints
def bounds_constraint_rule(model, i):
    lb, ub = investment_bounds[i]
    return (lb, model.x[i], ub)

model.bounds_constraint = Constraint(model.assets, rule=bounds_constraint_rule)

# Solve the model
solver = SolverFactory('scip')
solver.solve(model, tee=True)

# Display results
print("Optimal Portfolio:")
for i in model.assets:
    print(f"Asset {i}: {model.x[i].value * 100:.2f}%")

print(f"Expected Return: {model.expected_value():.4f}")