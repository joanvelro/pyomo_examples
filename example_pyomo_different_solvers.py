import os
import pyomo.environ as pyo
from pyomo.environ import SolverFactory

url = """https://ampl.com/dl/open/ipopt/ipopt-linux64.zip"""
os.system("wget -N -q {}".format(url))
# os.system("unzip -o -q ipopt-linux64")

# create a model
model = pyo.ConcreteModel()
# declare decision variables
model.x = pyo.Var(domain=pyo.NonNegativeReals)
model.y = pyo.Var(domain=pyo.NonNegativeReals)
# declare objective
model.profit = pyo.Objective(expr=40 * model.x + 30 * model.y, sense=pyo.maximize)
# declare constraints
model.demand = pyo.Constraint(expr=model.x <= 40)
model.laborA = pyo.Constraint(expr=model.x + model.y <= 80)
model.laborB = pyo.Constraint(expr=2 * model.x + model.y <= 100)
model.pprint()

# create a model
discrete_model = pyo.ConcreteModel()
# declare decision variables
discrete_model.x = pyo.Var(domain=pyo.NonNegativeIntegers)
discrete_model.y = pyo.Var(domain=pyo.NonNegativeIntegers)
# declare objective
discrete_model.profit = pyo.Objective(expr=40 * discrete_model.x + 30 * discrete_model.y, sense=pyo.maximize)
# declare constraints
discrete_model.demand = pyo.Constraint(expr=discrete_model.x <= 40)
discrete_model.laborA = pyo.Constraint(expr=discrete_model.x + discrete_model.y <= 80)
discrete_model.laborB = pyo.Constraint(expr=2 * discrete_model.x + discrete_model.y <= 100)
discrete_model.pprint()

# create a model
non_linear_model = pyo.ConcreteModel()
# declare decision variables
non_linear_model.x = pyo.Var(domain=pyo.NonNegativeReals)
non_linear_model.y = pyo.Var(domain=pyo.NonNegativeReals)
a = 1
b = 100
# declare objective
non_linear_model.rosenbrock = pyo.Objective(
    expr=(a - non_linear_model.x) ** 2 + b * (non_linear_model.y - non_linear_model.x) ** 2,
    sense=pyo.minimize)
# declare constraints
non_linear_model.constraint_1 = pyo.Constraint(expr=non_linear_model.x <= 2)
non_linear_model.constraint_2 = pyo.Constraint(expr=non_linear_model.x >= -2)
non_linear_model.constraint_3 = pyo.Constraint(expr=non_linear_model.x + non_linear_model.y <= 100)
non_linear_model.pprint()

# create a model
non_linear_model2 = pyo.ConcreteModel()

# declare decision variables
non_linear_model2.x1 = pyo.Var(domain=pyo.NonNegativeReals)
non_linear_model2.x2 = pyo.Var(domain=pyo.NonNegativeReals)
non_linear_model2.x3 = pyo.Var(domain=pyo.NonNegativeReals)

a1 = 150
a2 = 100
a3 = 340

# declare objective
non_linear_model2.cost = pyo.Objective(expr=(non_linear_model2.x1 ** 2) * a1 +
                                            (non_linear_model2.x2 ** 2) * a2 +
                                            (non_linear_model2.x3 ** 2) * a3,
                                       sense=pyo.minimize)
# declare constraints
non_linear_model2.constraint_1 = pyo.Constraint(
    expr=non_linear_model2.x1 + non_linear_model2.x2 + non_linear_model2.x3 == 300)
non_linear_model2.constraint_2 = pyo.Constraint(expr=non_linear_model2.x1 <= 80)
non_linear_model2.constraint_3 = pyo.Constraint(expr=non_linear_model2.x3 >= 50)
non_linear_model2.pprint()

print('\n')
print('---------------')
print('IPOPT')
print('---------------')
SolverFactory('ipopt').solve(model, tee=True,  logfile='ipopt_model.log')#.write()

# display solution
print('\nProfit = ', model.profit())

print('\nDecision Variables')
print('x = ', model.x())
print('y = ', model.y())

print('\nConstraints')
print('Demand  = ', model.demand())
print('Labor A = ', model.laborA())
print('Labor B = ', model.laborB())

#  executable='/usr/bin/glpsol
print('\n')
print('---------------')
print('GLPK')
print('---------------')
SolverFactory('glpk').solve(model, tee=True,  logfile='glpk_model.log') #.write()

# display solution
print('\nProfit = ', model.profit())

print('\nDecision Variables')
print('x = ', model.x())
print('y = ', model.y())

print('\nConstraints')
print('Demand  = ', model.demand())
print('Labor A = ', model.laborA())
print('Labor B = ', model.laborB())

#  executable=''/usr/bin/cbc'
print('\n')
print('---------------')
print('CBC')
print('---------------')
SolverFactory('cbc').solve(model, tee=True,  logfile='cbc_model.log') #.write()

# display solution
print('\nProfit = ', model.profit())

print('\nDecision Variables')
print('x = ', model.x())
print('y = ', model.y())

print('\nConstraints')
print('Demand  = ', model.demand())
print('Labor A = ', model.laborA())
print('Labor B = ', model.laborB())