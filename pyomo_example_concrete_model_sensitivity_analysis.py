# Import Pyomo and sipopt from the sensitivity toolbox

import pyomo.environ as pyo
from pyomo.contrib.sensitivity_toolbox.sens import sipopt
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# Create a concrete model

m = pyo.ConcreteModel()

# Define the variables with bounds and initial values

m.x1 = pyo.Var(initialize=0.15, within=pyo.NonNegativeReals)

m.x2 = pyo.Var(initialize=0.15, within=pyo.NonNegativeReals)

m.x3 = pyo.Var(initialize=0.0, within=pyo.NonNegativeReals)

# Define the parameters

m.eta1 = pyo.Param(initialize=4.5, mutable=True)

m.eta2 = pyo.Param(initialize=1.0, mutable=True)

# Define the constraints and objective

m.const1 = pyo.Constraint(expr=6 * m.x1 + 3 * m.x2 + 2 * m.x3 - m.eta1 == 0)

m.const2 = pyo.Constraint(expr=m.eta2 * m.x1 + m.x2 - m.x3 - 1 == 0)

m.cost = pyo.Objective(expr=m.x1 ** 2 + m.x2 ** 2 + m.x3 ** 2)

m.perturbed_eta1 = pyo.Param(initialize=4.0)

m.perturbed_eta2 = pyo.Param(initialize=1.0)

m_sipopt = sipopt(m, [m.eta1, m.eta2], [m.perturbed_eta1, m.perturbed_eta2], streamSoln=True)
