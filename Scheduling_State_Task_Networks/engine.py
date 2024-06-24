import matplotlib.pyplot as plt
import logging
from pandas import DataFrame
from IPython.display import display, HTML

import shutil
import sys
import os.path

logger = logging.getLogger(__name__)

from pyomo.environ import *
import pyomo.environ as pyo
from model_data import ModelData


class Engine(object):

    def __init__(self, model_data: ModelData) -> None:
        self.state_inventories = None
        self.UnitAssignment = None
        self.Bmax = None
        self.Bmin = None
        self.P = None
        self.rho = None
        self.rho_ = None
        self.S_ = None
        self.T = None
        self.T_ = None
        self.C = None
        self.p = None
        self.I = None
        self.K = None
        self.UNIT_TASKS = None
        self.STATES = None
        self.TIME = None
        self.UNITS = None
        self.TASKS = None
        self.H = None
        self.model = None
        self.data = model_data

    def execute(self, verbosity: bool = True, solver: int = 'cbc'):
        try:
            self.build_parameters()
            self.build_model()

            self.number_variables = self.model.nvariables()
            self.number_constraints = self.model.nconstraints()
            self.number_objectives = self.model.nobjectives()
            print('Num. variables: {}'.format(self.number_variables))
            print('Num. constraints: {}'.format(self.number_constraints))
            print('Num. objectives: {}'.format(self.number_objectives))

            self.solve_model(verbosity, solver)
            self.calculate_results()
            logger.info('\nOptimization completed!')

        except Exception as err:
            print('There was an unexpected error running the local solver engine')
            raise Exception('There was an unexpected error running the local solver engine') from err

    def build_parameters(self):
        """
        TBD
        """
        logger.info('Building parameters...')
        self.TASKS = self.data.get_data_dict()['TASKS']
        self.UNITS = self.data.get_data_dict()['UNITS']
        self.TIME = self.data.get_data_dict()['TIME']
        self.STATES = self.data.get_data_dict()['STATES']
        self.UNIT_TASKS = self.data.get_data_dict()['UNIT_TASKS']
        self.K = self.data.get_data_dict()['K']
        self.I = self.data.get_data_dict()['I']
        self.p = self.data.get_data_dict()['p']
        self.C = self.data.get_data_dict()['C']
        self.T_ = self.data.get_data_dict()['T_']
        self.S_ = self.data.get_data_dict()['S_']
        self.T = self.data.get_data_dict()['T']
        self.rho = self.data.get_data_dict()['rho']
        self.rho_ = self.data.get_data_dict()['rho_']
        self.P = self.data.get_data_dict()['P']
        self.Bmin = self.data.get_data_dict()['Bmin']
        self.Bmax = self.data.get_data_dict()['Bmax']
        self.H = self.data.get_data_dict()['H']

    def build_model(self):
        """
        TODO
        """
        logger.info('Building model...')
        self.model = ConcreteModel()

        # W[i,j,t] 1 if task i starts in unit j at time t
        self.model.W = Var(self.TASKS, self.UNITS, self.TIME, domain=pyo.Boolean)

        # B[i,j,t,] size of batch assigned to task i in unit j at time t
        self.model.B = Var(self.TASKS, self.UNITS, self.TIME, domain=pyo.NonNegativeReals)

        # S[s,t] inventory of state s at time t
        self.model.S = Var(self.STATES.keys(), self.TIME, domain=pyo.NonNegativeReals)

        # Q[j,t] inventory of unit j at time t
        self.model.Q = Var(self.UNITS, self.TIME, domain=pyo.NonNegativeReals)

        # Objective function
        # project value
        self.model.Value = Var(domain=pyo.NonNegativeReals)
        self.model.valuec = \
            Constraint(
                expr=self.model.Value == sum([self.STATES[s]['price'] * self.model.S[s, self.H] for s in self.STATES]))

        # project cost
        self.model.Cost = Var(domain=pyo.NonNegativeReals)
        self.model.costc = \
            Constraint(expr=self.model.Cost == sum([self.UNIT_TASKS[(j, i)]['Cost'] * self.model.W[i, j, t] +
                                                    self.UNIT_TASKS[(j, i)]['vCost'] * self.model.B[i, j, t]
                                                    for i in self.TASKS for j in self.K[i] for t in self.TIME]))

        self.model.obj = Objective(expr=self.model.Value - self.model.Cost, sense=maximize)

        # Constraints
        self.model.cons = ConstraintList()

        # a unit can only be allocated to one task
        for j in self.UNITS:
            for t in self.TIME:
                lhs = 0
                for i in self.I[j]:
                    for tprime in self.TIME:
                        if (t - self.p[i] + 1 - self.UNIT_TASKS[(j, i)]['Tclean']) <= tprime <= t:
                            lhs += self.model.W[i, j, tprime]
                self.model.cons.add(lhs <= 1)

        # state capacity constraint
        self.model.sc = Constraint(self.STATES.keys(), self.TIME, rule=lambda model, s, t: model.S[s, t] <= self.C[s])

        # state mass balances
        for s in self.STATES.keys():
            rhs = self.STATES[s]['initial']
            for t in self.TIME:
                for i in self.T_[s]:
                    for j in self.K[i]:
                        if t >= self.P[(i, s)]:
                            rhs += self.rho_[(i, s)] * self.model.B[i, j, max(self.TIME[self.TIME <= t - self.P[(i, s)]]
                                                                              )]
                for i in self.T[s]:
                    rhs -= self.rho[(i, s)] * sum([self.model.B[i, j, t] for j in self.K[i]])
                self.model.cons.add(self.model.S[s, t] == rhs)
                rhs = self.model.S[s, t]

        # unit capacity constraints
        for t in self.TIME:
            for j in self.UNITS:
                for i in self.I[j]:
                    self.model.cons.add(self.model.W[i, j, t] * self.Bmin[i, j] <= self.model.B[i, j, t])
                    self.model.cons.add(self.model.B[i, j, t] <= self.model.W[i, j, t] * self.Bmax[i, j])

        # unit mass balances
        for j in self.UNITS:
            rhs = 0
            for t in self.TIME:
                rhs += sum([self.model.B[i, j, t] for i in self.I[j]])
                for i in self.I[j]:
                    for s in self.S_[i]:
                        if t >= self.P[(i, s)]:
                            rhs -= self.rho_[(i, s)] * self.model.B[i, j, max(self.TIME[self.TIME <= t - self.P[(i, s)]]
                                                                              )]
                self.model.cons.add(self.model.Q[j, t] == rhs)
                rhs = self.model.Q[j, t]

        # unit terminal condition
        self.model.tc = Constraint(self.UNITS, rule=lambda model, j: model.Q[j, self.H] == 0)

    def solve_model(self, verbosity: bool = True, solver: int = 'cbc'):
        """
        TBD

        """
        logger.info('Solving Model...')

        solver = 'cplex'
        executable = '/Applications/CPLEX_Studio_Community2211/cplex/bin/x86-64_osx/cplex'
        # opt = SolverFactory(solver,  executable =executable )  # glpk
        #opt = pyo.SolverFactory(solver, executable=executable)


        if solver=='cbc':
            opt = SolverFactory(solver)
            opt.options['integertolerance'] = 1e-6
            opt.options['ratioGAP'] = 0.005
            opt.options['timeMode'] = 'elapsed'
            opt.options['seconds'] = 1000
        elif solver=='cplex':
            opt = SolverFactory(solver, executable=executable)
            opt.options["timelimit"] = 300
            opt.options["mipgap"] = 0.01
        opt.solve(self.model, tee=verbosity).write()

        print('\n')
        print('\n')
        print("Value of State Inventories = {0:12.2f}".format(self.model.Value()))
        print("  Cost of Unit Assignments = {0:12.2f}".format(self.model.Cost()))
        print("             Net Objective = {0:12.2f}".format(self.model.Value() - self.model.Cost()))

        logger.info('\t Done!...')

    def get_UnitAssignment(self):

        return self.UnitAssignment

    def get_state_inventories(self):

        return self.state_inventories

    def calculate_results(self):
        """
        TBD
        """

        logger.info('Computing results')
        self.UnitAssignment = DataFrame({j: [None for t in self.TIME] for j in self.UNITS}, index=self.TIME)

        for t in self.TIME:
            for j in self.UNITS:
                for i in self.I[j]:
                    for s in self.S_[i]:
                        if t - self.p[i] >= 0:
                            if self.model.W[i, j, max(self.TIME[self.TIME <= t - self.p[i]])]() > 0:
                                self.UnitAssignment.loc[t, j] = None
                for i in self.I[j]:
                    if self.model.W[i, j, t]() > 0:
                        self.UnitAssignment.loc[t, j] = (i, self.model.B[i, j, t]())

        self.state_inventories = DataFrame([[self.model.S[s, t]() for s in self.STATES.keys()] for t in self.TIME],
                                           columns=self.STATES.keys(),
                                           index=self.TIME)

        logger.info('\t Done!')

    def plot_results(self):
        """
        TODO
        """

        plt.figure(figsize=(10, 6))
        for (s, idx) in zip(self.STATES.keys(), range(0, len(self.STATES.keys()))):
            plt.subplot(ceil(len(self.STATES.keys()) / 3), 3, idx + 1)
            tlast, ylast = 0, self.STATES[s]['initial']
            for (t, y) in zip(list(self.TIME), [self.model.S[s, t]() for t in self.TIME]):
                plt.plot([tlast, t, t], [ylast, ylast, y], 'b')
                # plt.plot([tlast,t],[ylast,y],'b.',ms=10)
                tlast, ylast = t, y
            plt.ylim(0, 1.1 * self.C[s])
            plt.plot([0, self.H], [self.C[s], self.C[s]], 'r--')
            plt.title(s)
        plt.tight_layout()
        plt.show()
