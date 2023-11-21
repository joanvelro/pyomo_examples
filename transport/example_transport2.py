""" Solve a transportation problem using pyomo """
import pandas as pd
import pyomo.environ as pyo
import numpy as np
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition


class Engine(object):

    def __init__(self, c, a, b):
        self.results = None
        self.model = None
        self.c = c
        self.a = a
        self.b = b
    def execute(self):

        self.build_model()
        self.solve_problem()
        return self.results

    def build_model(self):
        """
        Build model
        """
        self.model = pyo.ConcreteModel()

        self.model.x = pyo.Var(list(range(len(self.c))), domain=pyo.NonNegativeReals)

        self.model.OBJ = pyo.Objective(expr=sum(self.c[i] * self.model.x[i] for i in self.model.x),
                                       sense=pyo.minimize)

        self.model.Constraint1 = \
            pyo.Constraint(expr=sum(self.a[i] * self.model.x[i] for i in self.model.x) >= self.b)

        print(self.model.pprint())

    def solve_problem(self, solver: str = 'glpk', verbose: bool= True):
        opt = SolverFactory(solver)
        self.results = opt.solve(self.model, tee=verbose)


if __name__ == "__main__":
    # define input data
    c = [30, 40, 50, 60, 70]  # costs of components
    a = [0.05, 0.18, 0.25, 0.28, 0.33]  # capacity production
    b = 100  # demand of final product

    # Build and solve model
    results = Engine(a=a, b=b, c=c).execute()

    print('\n --- Solver results --- \n')
    print(results)

    # send results to stdout
    # results.write()
