#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
    This script ...

	Python Formulation for the Pyomo Modeler
    Example for sensitivity analysis
    J M Garrido, September 2014. File sensit1.py
    Usage: pyomo sensit1.py
"""

import pyomo.environ as pyo

# Data for Linear Optimization Problem
x = [1, 2, 3]
c = [45, 34, 29]
# Concrete Model
model = pyo.ConcreteModel(name="Sensitivity 1")

# for access to dual solution for constraints
model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# Decision Variables
model.x = pyo.Var(x, within=pyo.NonNegativeReals)

# The objective function
model.obj = pyo.Objective(expr=sum(c[i - 1] * model.x[i] for i in x), sense=pyo.maximize)

# Constraints
model.Constraint1 = pyo.Constraint(expr=8.0 * model.x[1] + 6.0 * model.x[2] + 8.3 * model.x[3] <= 48.0)
model.Constraint2 = pyo.Constraint(expr=4.0 * model.x[1] + 2.0 * model.x[2] + 1.5 * model.x[3] <= 20.0)
model.Constraint3 = pyo.Constraint(expr=6.0 * model.x[1] + 3.0 * model.x[2] + 1.5 * model.x[3] <= 60.0)

from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

results = pyo.SolverFactory('glpk').solve(model)



# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())

    print('--- Objective  ---')
    print('OF:', pyo.value(model.obj))
    print(model.obj.pprint())
    print('--- Constraint 1 ---')
    print('C1:', model.Constraint1())
    print(model.Constraint1.pprint())
    print('--- Constraint 2 ---')
    print('C2:', model.Constraint2())
    print(model.Constraint2.pprint())
    print('--- Constraint 3 ---')
    print('C3:', model.Constraint3())
    print(model.Constraint3.pprint())

    print('--- Decision variable---')
    print(model.x.pprint())
    print('--- Dual Variables ---')
    print(model.dual.pprint())
    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('(!) Infeasible solution')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
