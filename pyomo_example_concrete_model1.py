import pyomo.environ as pyo
from numpy import array
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

model = pyo.ConcreteModel()

n = 5
c = [30, 40, 50, 60, 70]
a = [0.05, 0.18, 0.25, 0.28, 0.33]
b = 100
d = [30, 20, 50, 80, 90]

x_0 = {0: 0, 1: 0, 2: 0, 3: 0, 4: 0}
# x_0 = {0:1000, 1:1000, 2:5000, 3:200, 4:4000}

model.i = pyo.Set(initialize=range(n))

model.x = pyo.Var(model.i, domain=pyo.NonNegativeReals, bounds=(0, 1000000), initialize=x_0)

model.OBJ = pyo.Objective(expr=sum(c[i] * model.x[i] for i in model.i), sense=pyo.minimize)

model.Constraint1 = pyo.Constraint(expr=sum(a[i] * model.x[i] for i in model.i) >= b)


def rule_exp(model, i):
    return model.x[i] >= d[i]


model.Constraint2 = pyo.Constraint(model.i, rule=rule_exp)

print(model.pprint())

opt = SolverFactory("glpk")

results = opt.solve(model, tee=False)  # tee=True for details

# send results to stdout
results.write()
# !cat results.yml


# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())
    print('--- Constraint ---')
    print('Ax:', model.Constraint1())
    print('--- Objective ---')
    print('z:', model.OBJ())
    print('--- Decision variable---')
    print('x(1):', pyo.value(model.x[0]))
    print('x(2):', pyo.value(model.x[1]))
    print('x(3):', pyo.value(model.x[2]))
    print('x(4):', pyo.value(model.x[3]))
    print('x(5):', pyo.value(model.x[4]))

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
