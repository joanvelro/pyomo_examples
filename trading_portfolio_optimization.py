"""
In this portfolio optimization model, the set model.I represents the assets in the portfolio.
The parameters model.return_rate, model.variance, and model.covariance represent the expected return rate, variance,
 and covariance, respectively, of each asset. The parameter model.budget represents the total budget available for the
 portfolio.

The decision variable model.x represents the proportion of each asset in the portfolio. The objective
 function aims to maximize the expected return of the portfolio. The budget constraint ensures that the total
 proportion of assets in the portfolio does not exceed the available budget.

Make sure to replace <solver_name> with the name of the solver you have installed (e.g., glpk, cplex, gurobi)
 before running the code.

"""
from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Sets
model.I = Set()  # Set of assets

# Parameters
model.return_rate = Param(model.I)  # Expected return rate of each asset
model.variance = Param(model.I)  # Variance of each asset
model.covariance = Param(model.I, model.I)  # Covariance between asset i and asset j
model.budget = Param()  # Total budget for the portfolio

# Decision Variables
model.x = Var(model.I, within=NonNegativeReals)  # Proportion of each asset in the portfolio

# Objective Function
def obj_rule(model):
    return sum(model.return_rate[i] * model.x[i] for i in model.I)
model.obj = Objective(rule=obj_rule, sense=maximize)

# Constraints
def budget_constraint_rule(model):
    return sum(model.x[i] for i in model.I) == model.budget
model.budget_constraint = Constraint(rule=budget_constraint_rule)

# Solve the model
solver = SolverFactory('<solver_name>')  # Replace <solver_name> with the name of the solver you want to use
solver.solve(model)

# Print the results
print(f"Optimal Portfolio Return: {model.obj()}")
for i in model.I:
    print(f"Proportion of asset {i}: {model.x[i].value}")