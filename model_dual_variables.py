import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
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
results = pyo.SolverFactory('ipopt').solve(model)

str = "   {0:7.2f} {1:7.2f} {2:7.2f} {3:7.2f}"

print(model.x.pprint())
print(model.y.pprint())

# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::::::::::::::::::::::')
    print(':  Feasible solution  : ')
    print(':::::::::::::::::::::::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print(':::::::::::::::::::::::')
    print(':  Infeasible solution  : ')
    print(':::::::::::::::::::::::')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)

print("Constraint  value  lslack  uslack    dual")
for constraint in [model.demand, model.laborA, model.laborB]:
    print(constraint, str.format(constraint(), constraint.lslack(), constraint.uslack(), model.dual[constraint]))


for c in model.component_objects(ctype=pyo.Constraint):
    if c.slack() == 0:  # constraint is not met
        print(f'Constraint {c.name} is not satisfied')
        c.display()  # show the evaluation of c
        c.pprint()   # show the construction of c
        print()
else:
    pass
    # instantiate solver & solve, etc...