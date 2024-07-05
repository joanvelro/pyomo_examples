#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
.. module:: scheduling_LS
   :synopsis:  This script define the Scheduling optimization problem using LocalSolver

.. moduleauthor:: Jose Angel Velasco - (C) Repsol Data Hub - 2021
"""

import localsolver
import pandas

with localsolver.LocalSolver() as ls:
    #
    # Declares the optimization model
    #
    model = ls.model

    # ==================
    # ::: constants :::
    # ==================

    # maximum capacity
    q_c_max = 25000  # components tank
    q_b_max = 25000  # blending tank
    q_s_max = 25000  # stock tank

    # maximum capacity
    q_c_min = 500  # stock tank
    q_b_min = 500  # blending tank
    q_s_min = 500  # stock tank

    # initial stocks
    q_c_0 = 5000
    q_s_0 = 5000
    q_b_0 = 5000

    # ==================
    # ::: Sets :::
    # ==================

    # components tank : 1
    # blending tank: 2
    # final product tank: 3

    # ==================
    # ::: Parameters :::
    # ==================

    # Demand to supply
    demand = [1000, 800, 900, 700, 1000, 1200, 1500, 1600, 1200]

    # time periods (days)
    T = len(demand)  # 9

    # blending_time # days
    L = 2

    # minimum blending quantity
    qbs = 5000

    # minimum quantity to purchase
    x_min = 250
    x_max = 10000

    flow_rate = 1000

    # =================
    # ::: Variables :::
    # =================

    # binary variable for blending
    y_bs = [model.bool() for i in range(T)]

    # binary variable for minimum purchase
    y_cb = [model.bool() for i in range(T)]

    # quantity components tank
    qc = [model.int(q_c_min, q_c_max) for i in range(T)]

    # quantity blending tank
    qb = [model.int(q_b_min, q_b_max) for i in range(T)]

    # quantity stock tank
    qs = [model.int(q_s_min, q_s_max) for i in range(T)]

    # flow from components to blending
    f_cb = [model.int(0, 10000) for i in range(T)]

    # quantity stock tank
    f_bs = [model.int(0, 10000) for i in range(T)]

    # quantity to purchase
    x = [model.int(x_min, x_max) for i in range(T)]

    # quantity to purchase
    z = [model.int(0, 10000) for i in range(T)]

    # ==========================
    # ::: Constraints  :::
    # ==========================

    for t in range(0, T-1):
        try:
            # intialize levels
            if t == 0:
                model.constraint(qs[t] == q_s_0)
                model.constraint(qc[t] == q_c_0)
                model.constraint(qb[t] == q_b_0)
            # balance
            if t < T:
                model.constraint(qs[t + 1] == qs[t] - demand[t] + f_bs[t])
                model.constraint(qb[t + 1] == qb[t] + f_cb[t] - f_bs[t])
                model.constraint(qc[t + 1] == qc[t] + x[t] - f_cb[t])

            model.constraint(qs[t] >= demand[t])
            model.constraint(f_bs[t] <= qb[t])
            model.constraint(f_cb[t] <= qc[t])
            model.constraint(f_bs[t] == flow_rate * y_bs[t])
            model.constraint(f_cb[t] == flow_rate * y_cb[t])
            model.constraint(y_cb[t] + y_bs[t] == 1)

        except Exception as exception_msg:
            print('(!) Error in constraints: {} period:{}'.format(str(exception_msg), t))

    # ==========================
    # ::: Objective function :::
    # ==========================

    # dummy objective function
    #dummmy_OF = 1 + 1
    dummmy_OF = sum(z[t] for t in range(T))
    # model.minimize(sum(100*x[t] + 20*f_ds[t] for t in range(T)))
    model.minimize(dummmy_OF)

    model.close()

    # ::: model params :::

    # ls.param.time_limit = 5
    ls.param.iteration_limit = 2000000

    # ::: Solve model :::
    ls.solve()

    # ::: Results :::

    df_results = pandas.DataFrame({
        't': [i for i in range(T)],
        'qb': [qb[i].value for i in range(T)],
        'qc': [qc[i].value for i in range(T)],
        'qs': [qs[i].value for i in range(T)],
        'f_cb': [f_cb[i].value for i in range(T)],
        'f_bs': [f_bs[i].value for i in range(T)],
        'y_cb': [y_cb[i].value for i in range(T)],
        'y_bs': [y_bs[i].value for i in range(T)],
        'x': [x[i].value for i in range(T)],
        'demand': demand})
