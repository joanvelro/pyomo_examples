from pyomo.environ import *

# Define the model
model = ConcreteModel()

# Sets and Parameters
num_assets = 3  # Number of assets
num_markets = 2  # Number of markets

# Indices
model.assets = RangeSet(1, num_assets)
model.markets = RangeSet(1, num_markets)

# Parameters (example data)
asset_prices = {1: 100, 2: 50, 3: 80}  # Example asset prices
market_caps = {1: 5000, 2: 3000}  # Example market capitalizations

# Variables
model.x = Var(model.assets, model.markets, within=NonNegativeReals)

# Objective function
model.total_value = Objective(
    expr=sum(asset_prices[i] * model.x[i, j] for i in model.assets for j in model.markets),
    sense=maximize
)

# Constraints
def market_cap_constraint_rule(model, j):
    return sum(model.x[i, j] for i in model.assets) <= market_caps[j]

model.market_cap_constraint = Constraint(model.markets, rule=market_cap_constraint_rule)

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model, tee=True)

# Display results
print("Optimal Trading Plan:")
for j in model.markets:
    print(f"Market {j}:")
    for i in model.assets:
        print(f"  Asset {i}: {model.x[i, j].value}")

print(f"Total Portfolio Value: {model.total_value():.2f}")