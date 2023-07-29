import pyomo.environ as pyo
from pyomo.environ import SolverFactory
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition


# INPUT DATA
# ------------------------------------------------------
# conversion coefficients
beta_1 = 0.9
beta_2 = 0.82
beta_3 = 0.95

# limits
AI_max = 45
AI_min = 2

BI_min = 2
BI_max = 40

BII_min = 2
BII_max = 40

BIII_min = 2
BIII_max = 35

Bext_min = 2
Bext_max = 50

# demand
demand = 33

# objective parameters

profit_coef = 1800

cost_AI = 500
cost_Bext = 950

aI = 1000
aII = 2200
aIII = 1500

bI = 250
bII = 400
bIII = 550
# ------------------------------------------------------


model = pyo.ConcreteModel()
model.yI = pyo.Var(within=pyo.Binary)
model.yII = pyo.Var(within=pyo.Binary)
model.yIII = pyo.Var(within=pyo.Binary)

model.AI = pyo.Var(domain=pyo.NonNegativeReals)
model.Bext = pyo.Var(domain=pyo.NonNegativeReals)
model.BI = pyo.Var(domain=pyo.NonNegativeReals)
model.BII = pyo.Var(domain=pyo.NonNegativeReals)
model.BIII = pyo.Var(domain=pyo.NonNegativeReals)
model.CI = pyo.Var(domain=pyo.NonNegativeReals)
model.CII = pyo.Var(domain=pyo.NonNegativeReals)
model.CIII = pyo.Var(domain=pyo.NonNegativeReals)

model.costeI = pyo.Var(domain=pyo.NonNegativeReals)
model.costeII = pyo.Var(domain=pyo.NonNegativeReals)
model.costeIII = pyo.Var(domain=pyo.NonNegativeReals)


# conversion
model.constI2 = pyo.Constraint(expr=model.BI == beta_1*model.AI)
model.constII2 = pyo.Constraint(expr=model.CII == beta_2*model.BII)
model.constIII2 = pyo.Constraint(expr=model.CIII == beta_3*model.BIII)


# technical limis
model.AI_max = pyo.Constraint(expr=model.AI <= AI_max*model.yI)
model.BII_max = pyo.Constraint(expr=model.BII <= BII_max*model.yII)
model.BIII_max = pyo.Constraint(expr=model.BIII <= BIII_max*model.yIII)
model.Bext_max = pyo.Constraint(expr=model.Bext <= Bext_max)

model.AI_min = pyo.Constraint(expr=model.AI >= AI_min*model.yI)
model.BII_min = pyo.Constraint(expr=model.BII >= BII_min*model.yII)
model.BIII_min = pyo.Constraint(expr=model.BIII >= BIII_min*model.yIII)
model.Bext_min = pyo.Constraint(expr=model.Bext >= Bext_min)

# balance
model.balance_c = pyo.Constraint(expr=model.CII + model.CIII == model.CI)
model.balance_b = pyo.Constraint(expr=model.BI + model.Bext == model.BII + model.BIII)

# logic
model.logic_process = pyo.Constraint(expr=model.yII + model.yIII == 1)

# demand
model.demand = pyo.Constraint(expr=model.CI == demand)


# cost expressions
model.constI1 = pyo.Constraint(expr=model.costeI == bI * model.AI + aI * model.yI)
model.constII1 = pyo.Constraint(expr=model.costeII == bII * model.BII + aII * model.yII)
model.constIII1 = pyo.Constraint(expr=model.costeIII == bIII * model.BIII + aIII * model.yIII)

# objective
model.obj = pyo.Objective(expr=
                          profit_coef*model.CI - cost_AI*model.AI - cost_Bext*model.Bext - model.costeI -
                          model.costeII - model.costeIII,
                          sense=pyo.maximize)

opt = SolverFactory("cbc")

results = opt.solve(model, tee=True)  # tee=True for details

print('\n')
print('\n')
print('--- Model Results ---')


# Check the solution
if (results.solver.status == SolverStatus.ok) and (results.solver.termination_condition == TerminationCondition.optimal):
    print('\n')
    print('--- Status: OPTIMAL -- ')
    print('\n')
    print('--- Objective ---')
    print('Profit:', pyo.value(model.obj))
    print('\n')
    print('--- Variables ---')
    print('yI:', pyo.value(model.yI))
    print('yII:', pyo.value(model.yII))
    print('yIII:', pyo.value(model.yIII))
    print('Bext:', pyo.value(model.Bext))
    print('AI:', pyo.value(model.AI))
    print('BI:', pyo.value(model.BI))
    print('BII:', pyo.value(model.BII))
    print('BIII:', pyo.value(model.BIII))
    print('CI:', pyo.value(model.CI))
    print('CII:', pyo.value(model.CII))
    print('CIII:', pyo.value(model.CIII))

elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('\n (!) Status: INFEASIBLE')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
