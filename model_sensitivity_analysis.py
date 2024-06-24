
"""
The sensitivity toolbox provides a Pyomo interface to sIPOPT and k_aug to very quickly compute approximate solutions to
nonlinear programs with a small perturbation in model parameters.

"""
from pyomo.environ import *

from pyomo.contrib.sensitivity_toolbox.sens import sensitivity_calculation

# Create a concrete model

m = ConcreteModel()

# Define the variables with bounds and initial values

m.x1 = Var(initialize=0.15, within=NonNegativeReals)

m.x2 = Var(initialize=0.15, within=NonNegativeReals)

m.x3 = Var(initialize=0.0, within=NonNegativeReals)

# Define the parameters

m.eta1 = Param(initialize=4.5, mutable=True)

m.eta2 = Param(initialize=1.0, mutable=True)

# Define the constraints and objective

m.const1 = Constraint(expr=6 * m.x1 + 3 * m.x2 + 2 * m.x3 - m.eta1 == 0)

m.const2 = Constraint(expr=m.eta2 * m.x1 + m.x2 - m.x3 - 1 == 0)

m.cost = Objective(expr=m.x1 ** 2 + m.x2 ** 2 + m.x3 ** 2)

m.perturbed_eta1 = Param(initialize=4.0)

m.perturbed_eta2 = Param(initialize=1.0)

m_sipopt = sensitivity_calculation(method='sipopt',
                                   instance=m,
                                   paramList=[m.eta1, m.eta2],
                                   perturbList=[m.perturbed_eta1, m.perturbed_eta2],
                                   tee=True)

print("eta1 = %0.3f" % m.eta1())

print("eta2 = %0.3f" % m.eta2())

# Initial point (not feasible):

print("Objective = %0.3f" % m.cost())

print("x1 = %0.3f" % m.x1())

print("x2 = %0.3f" % m.x2())

print("x3 = %0.3f" % m.x3())

# Solution with the original parameter values:

print("Objective = %0.3f" % m_sipopt.cost())


print("x1 = %0.3f" % m_sipopt.x1())


print("x2 = %0.3f" % m_sipopt.x2())


print("x3 = %0.3f" % m_sipopt.x3())
