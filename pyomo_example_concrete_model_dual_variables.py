import pyomo.environ as pyo
import numpy as np

model = pyo.ConcreteModel()

# for access to dual solution for constraints
model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# declare decision variables
model.x = pyo.Var(domain=pyo.NonNegativeReals)
model.y = pyo.Var(domain=pyo.NonNegativeReals)

# declare objective
model.profit = pyo.Objective(expr=model.x**2 + model.y**3, sense=pyo.maximize)

# declare constraints
model.demand = pyo.Constraint(expr=model.x <= 40)
model.laborA = pyo.Constraint(expr=model.x + model.y <= 80)
model.laborB = pyo.Constraint(expr=2 * model.x + model.y <= 100)

print(model.pprint())

# solve
pyo.SolverFactory('ipopt').solve(model).write()

str = "   {0:7.2f} {1:7.2f} {2:7.2f} {3:7.2f}"

print(model.x.pprint())
print(model.y.pprint())

print("Constraint  value  lslack  uslack    dual")
for constraint in [model.demand, model.laborA, model.laborB]:
    print(constraint, str.format(constraint(), constraint.lslack(), constraint.uslack(), model.dual[constraint]))
