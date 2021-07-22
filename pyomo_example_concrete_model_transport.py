#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    This script ...

"""

import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# Data
# product demand
Demand = {
    'Lon': 125,  # London
    'Ber': 175,  # Berlin
    'Maa': 225,  # Maastricht
    'Ams': 250,  # Amsterdam
    'Utr': 225,  # Utrecht
    'Hag': 200  # The Hague
}
# production capacity
Capacity = {
    'Arn': 600,  # Arnhem
    'Gou': 650  # Gouda
}

# transprot cost
Cost = {
    ('Lon', 'Arn'): 1000,
    ('Lon', 'Gou'): 2.5,
    ('Ber', 'Arn'): 2.5,
    ('Ber', 'Gou'): 1000,
    ('Maa', 'Arn'): 1.6,
    ('Maa', 'Gou'): 2.0,
    ('Ams', 'Arn'): 1.4,
    ('Ams', 'Gou'): 1.0,
    ('Utr', 'Arn'): 0.8,
    ('Utr', 'Gou'): 1.0,
    ('Hag', 'Arn'): 1.4,
    ('Hag', 'Gou'): 0.8
}




# Step 0: Create an instance of the model
model = pyo.ConcreteModel()

model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# Step 1: Define index sets
CUS = list(Demand.keys())
SRC = list(Capacity.keys())

# Step 2: Define the decision
model.x = pyo.Var(CUS, SRC, domain=pyo.NonNegativeReals)

# Step 3: Define Objective
model.OF = pyo.Objective(
    expr=sum([Cost[c, s] * model.x[c, s] for c in CUS for s in SRC]),
    sense=pyo.minimize)

# Step 4: Constraints
model.src = pyo.ConstraintList()
for s in SRC:
    model.src.add(sum([model.x[c, s] for c in CUS]) <= Capacity[s])

model.dmd = pyo.ConstraintList()
for c in CUS:
    model.dmd.add(sum([model.x[c, s] for s in SRC]) == Demand[c])

print(':::::: Model Overview ::::::')
print('\n')
print(model.pprint())

results = pyo.SolverFactory('glpk').solve(model)
#results.write()


# Check the solution
if (results.solver.status == SolverStatus.ok) and (results.solver.termination_condition == TerminationCondition.optimal):
    print('Feasible solution')
    print('Decision variable')
    print(model.x.pprint())
    print('Objective Function')
    print(model.OF.pprint())

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)











