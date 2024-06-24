from pyomo.environ import (
    ConcreteModel,
    Var,
    Constraint,
    Objective,
    SolverFactory,
    value,
    NonNegativeReals,
    minimize,
    maximize)

# Define the model
model = ConcreteModel()

# Define the decision variables
model.x = Var(domain=NonNegativeReals, initialize=0)
model.y = Var(domain=NonNegativeReals, initialize=0)

# Define the objective (for the sake of example, let's minimize y)
model.obj = Objective(expr=model.y, sense=maximize)


# Define the constraints with conditional logic
def constraint_rule(model):
    if value(model.x) > 5:
        return model.y <= 2 * model.x
    else:
        return model.y <= 3 * model.x


model.constraint = Constraint(rule=constraint_rule)

# Optionally add bounds or other constraints
model.x.setlb(0)
model.x.setub(10)
model.y.setlb(0)
model.y.setub(30)

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model)

# Print the results
print(f"Optimal value of x: {value(model.x)}")
print(f"Optimal value of y: {value(model.y)}")
