"""
https://jckantor.github.io/ND-Pyomo-Cookbook/notebooks/04.05-Scheduling-Multipurpose-Batch-Processes-using-State-Task_Networks.html
"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from IPython.display import display, HTML

import shutil
import sys
import os.path

from pyomo.environ import *

# planning horizon
resolution = 15 # min
H = int((60/15)*24*7)  # 1 week, 15 min resolution , 15955 variables
# H = 12*24*7  # 1 week, 5 min resolution , 2016
# H = 60*24*7  # 1 week, 1 min resolution , 10080
# H = 60*24*7  # 1 day, 1 min resolution , 10080


Kondili = {
    # time grid
    'TIME': range(0, H + 1),

    # states
    'STATES': {
        'Feed_A': {'capacity': 500, 'initial': 500, 'price': 0},
        'Feed_B': {'capacity': 500, 'initial': 500, 'price': 0},
        'Feed_C': {'capacity': 500, 'initial': 500, 'price': 0},
        'Hot_A': {'capacity': 100, 'initial': 0, 'price': -100},
        'Int_AB': {'capacity': 200, 'initial': 0, 'price': -100},
        'Int_BC': {'capacity': 150, 'initial': 0, 'price': -100},
        'Impure_E': {'capacity': 100, 'initial': 0, 'price': -100},
        'Product_1': {'capacity': 500, 'initial': 300, 'price': 30},
        'Product_2': {'capacity': 500, 'initial': 0, 'price': 10},
    },

    # state-to-task arcs indexed by (state, task)
    'ST_ARCS': {
        ('Feed_A', 'Heating'): {'rho': 1.0},
        ('Feed_B', 'Reaction_1'): {'rho': 0.5},
        ('Feed_C', 'Reaction_1'): {'rho': 0.5},
        ('Feed_C', 'Reaction_3'): {'rho': 0.2},
        ('Hot_A', 'Reaction_2'): {'rho': 0.4},
        ('Int_AB', 'Reaction_3'): {'rho': 0.8},
        ('Int_BC', 'Reaction_2'): {'rho': 0.6},
        ('Impure_E', 'Separation'): {'rho': 1.0},
    },

    # task-to-state arcs indexed by (task, state)
    'TS_ARCS': {
        ('Heating', 'Hot_A'): {'dur': 60/resolution, 'rho': 1.0},
        ('Reaction_2', 'Product_1'): {'dur': 2*60/resolution, 'rho': 0.4},
        ('Reaction_2', 'Int_AB'): {'dur': 2*60/resolution, 'rho': 0.6},
        ('Reaction_1', 'Int_BC'): {'dur': 2*60/resolution, 'rho': 1.0},
        ('Reaction_3', 'Impure_E'): {'dur': 1*60/resolution, 'rho': 1.0},
        ('Separation', 'Int_AB'): {'dur': 2*60/resolution, 'rho': 0.1},
        ('Separation', 'Product_2'): {'dur': 1*60/resolution, 'rho': 0.9},
    },

    # unit data indexed by (unit, task)
    'UNIT_TASKS': {
        ('Heater', 'Heating'): {'Bmin': 0, 'Bmax': 100, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_1', 'Reaction_1'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_1', 'Reaction_2'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_1', 'Reaction_3'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_2', 'Reaction_1'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_2', 'Reaction_2'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Reactor_2', 'Reaction_3'): {'Bmin': 0, 'Bmax': 80, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
        ('Still', 'Separation'): {'Bmin': 0, 'Bmax': 200, 'Cost': 1, 'vCost': 0, 'Tclean': 0},
    },
}


def build_instance(STN):
    STATES = STN['STATES']
    ST_ARCS = STN['ST_ARCS']
    TS_ARCS = STN['TS_ARCS']
    UNIT_TASKS = STN['UNIT_TASKS']
    TIME = STN['TIME']

    # set of tasks
    TASKS = set([i for (j, i) in UNIT_TASKS])

    # S[i] input set of states which feed task i
    S = {i: set() for i in TASKS}
    for (s, i) in ST_ARCS:
        S[i].add(s)

    # S_[i] output set of states fed by task i
    S_ = {i: set() for i in TASKS}
    for (i, s) in TS_ARCS:
        S_[i].add(s)

    # rho[(i,s)] input fraction of task i from state s
    rho = {(i, s): ST_ARCS[(s, i)]['rho'] for (s, i) in ST_ARCS}

    # rho_[(i,s)] output fraction of task i to state s
    rho_ = {(i, s): TS_ARCS[(i, s)]['rho'] for (i, s) in TS_ARCS}

    # P[(i,s)] time for task i output to state s
    P = {(i, s): TS_ARCS[(i, s)]['dur'] for (i, s) in TS_ARCS}

    # p[i] completion time for task i
    p = {i: max([P[(i, s)] for s in S_[i]]) for i in TASKS}

    # K[i] set of units capable of task i
    K = {i: set() for i in TASKS}
    for (j, i) in UNIT_TASKS:
        K[i].add(j)

    # T[s] set of tasks receiving material from state s
    T = {s: set() for s in STATES}
    for (s, i) in ST_ARCS:
        T[s].add(i)

    # set of tasks producing material for state s
    T_ = {s: set() for s in STATES}
    for (i, s) in TS_ARCS:
        T_[s].add(i)

    # C[s] storage capacity for state s
    C = {s: STATES[s]['capacity'] for s in STATES}

    UNITS = set([j for (j, i) in UNIT_TASKS])

    # I[j] set of tasks performed with unit j
    I = {j: set() for j in UNITS}
    for (j, i) in UNIT_TASKS:
        I[j].add(i)

    # Bmax[(i,j)] maximum capacity of unit j for task i
    Bmax = {(i, j): UNIT_TASKS[(j, i)]['Bmax'] for (j, i) in UNIT_TASKS}

    # Bmin[(i,j)] minimum capacity of unit j for task i
    Bmin = {(i, j): UNIT_TASKS[(j, i)]['Bmin'] for (j, i) in UNIT_TASKS}

    return TIME, STATES, UNIT_TASKS, UNITS, Bmax, Bmin, I, T, T_, P, p, rho, rho_, S, S_, TASKS, K, C


def build_model(TIME, STATES, UNIT_TASKS, UNITS, Bmax, Bmin, I, T, T_, P, p, rho, rho_, S, S_, TASKS, K, C):
    print('building model...')
    TIME = np.array(TIME)

    model = ConcreteModel()

    # W[i,j,t] 1 if task i starts in unit j at time t
    model.W = Var(TASKS, UNITS, TIME, domain=Boolean)

    # B[i,j,t,] size of batch assigned to task i in unit j at time t
    model.B = Var(TASKS, UNITS, TIME, domain=NonNegativeReals)

    # S[s,t] inventory of state s at time t
    model.S = Var(STATES.keys(), TIME, domain=NonNegativeReals)

    # Q[j,t] inventory of unit j at time t
    model.Q = Var(UNITS, TIME, domain=NonNegativeReals)

    # Objective function

    # project value
    model.Value = Var(domain=NonNegativeReals)
    model.valuec = Constraint(expr=model.Value == sum([STATES[s]['price'] * model.S[s, H] for s in STATES]))

    # project cost
    model.Cost = Var(domain=NonNegativeReals)
    model.costc = Constraint(expr=model.Cost == sum([UNIT_TASKS[(j, i)]['Cost'] * model.W[i, j, t] +
                                                     UNIT_TASKS[(j, i)]['vCost'] * model.B[i, j, t] for i in TASKS for j
                                                     in K[i] for t in TIME]))

    model.obj = Objective(expr=model.Value - model.Cost, sense=maximize)

    # Constraints
    model.cons = ConstraintList()

    # a unit can only be allocated to one task
    for j in UNITS:
        for t in TIME:
            lhs = 0
            for i in I[j]:
                for tprime in TIME:
                    if tprime >= (t - p[i] + 1 - UNIT_TASKS[(j, i)]['Tclean']) and tprime <= t:
                        lhs += model.W[i, j, tprime]
            model.cons.add(lhs <= 1)

    # state capacity constraint
    model.sc = Constraint(STATES.keys(), TIME, rule=lambda model, s, t: model.S[s, t] <= C[s])

    # state mass balances
    for s in STATES.keys():
        rhs = STATES[s]['initial']
        for t in TIME:
            for i in T_[s]:
                for j in K[i]:
                    if t >= P[(i, s)]:
                        rhs += rho_[(i, s)] * model.B[i, j, max(TIME[TIME <= t - P[(i, s)]])]
            for i in T[s]:
                rhs -= rho[(i, s)] * sum([model.B[i, j, t] for j in K[i]])
            model.cons.add(model.S[s, t] == rhs)
            rhs = model.S[s, t]

            # unit capacity constraints
    for t in TIME:
        for j in UNITS:
            for i in I[j]:
                model.cons.add(model.W[i, j, t] * Bmin[i, j] <= model.B[i, j, t])
                model.cons.add(model.B[i, j, t] <= model.W[i, j, t] * Bmax[i, j])

                # unit mass balances
    for j in UNITS:
        rhs = 0
        for t in TIME:
            rhs += sum([model.B[i, j, t] for i in I[j]])
            for i in I[j]:
                for s in S_[i]:
                    if t >= P[(i, s)]:
                        rhs -= rho_[(i, s)] * model.B[i, j, max(TIME[TIME <= t - P[(i, s)]])]
            model.cons.add(model.Q[j, t] == rhs)
            rhs = model.Q[j, t]

    # unit terminal condition
    model.tc = Constraint(UNITS, rule=lambda model, j: model.Q[j, H] == 0)

    return model


def solve_model(model, solver: str = 'cbc', verbosity: str = 'True'):
    print('solving model...')
    opt = SolverFactory(solver)
    opt_parameters = {}
    opt.options.update(opt_parameters)

    if solver == "cbc":
        opt.options["ratioGAP"] = opt_parameters.get("ratioGAP", 0.005)
        opt.options["timeMode"] = opt_parameters.get("timeMode", "elapsed")
        opt.options["seconds"] = opt_parameters.get("seconds", 900)  # 15 min
        opt.options["integertolerance"] = opt_parameters.get("integertolerance", 1e-6)

    results = opt.solve(model, tee=verbosity)
    return results


def build_unit_assignment(TIME, UNITS, model, p, S_):
    UnitAssignment = pd.DataFrame({j: [None for t in TIME] for j in UNITS}, index=TIME)

    for t in TIME:
        for j in UNITS:
            for i in I[j]:
                for s in S_[i]:
                    if t - p[i] >= 0:
                        if value(model.W[i, j, t]) > 0:
                            UnitAssignment.loc[t, j] = None
            for i in I[j]:
                if model.W[i, j, t]() > 0:
                    UnitAssignment.loc[t, j] = (i, model.B[i, j, t]())

    return UnitAssignment


def build_state_inventories(model, STATES, TIME):
    return pd.DataFrame([[model.S[s, t]() for s in STATES.keys()] for t in TIME], columns=STATES.keys(), index=TIME)


def plot_results(STATES, model, TIME, C):
    plt.figure(figsize=(10, 6))
    for (s, idx) in zip(STATES.keys(), range(0, len(STATES.keys()))):
        plt.subplot(ceil(len(STATES.keys()) / 3), 3, idx + 1)
        tlast, ylast = 0, STATES[s]['initial']
        for (t, y) in zip(list(TIME), [model.S[s, t]() for t in TIME]):
            plt.plot([tlast, t, t], [ylast, ylast, y], 'b')
            # plt.plot([tlast,t],[ylast,y],'b.',ms=10)
            tlast, ylast = t, y
        plt.ylim(0, 1.1 * C[s])
        plt.plot([0, H], [C[s], C[s]], 'r--')
        plt.title(s)
    plt.tight_layout()
    plt.show()


TIME, STATES, UNIT_TASKS, UNITS, Bmax, Bmin, I, T, T_, P, p, rho, rho_, S, S_, TASKS, K, C = build_instance(STN=Kondili)

model = build_model(TIME, STATES, UNIT_TASKS, UNITS, Bmax, Bmin, I, T, T_, P, p, rho, rho_, S, S_, TASKS, K, C)

results = solve_model(model)

UnitAssignment = build_unit_assignment(TIME, UNITS, model, p, S_)

state_inventories = build_state_inventories(model, STATES, TIME)

plot_results(STATES, model, TIME, C)

print('hi')
