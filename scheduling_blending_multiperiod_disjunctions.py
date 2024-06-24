# 2016-Lotero
from typing import Any
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.express as px

from pandas import DataFrame, concat

import pyomo.environ as pyo
from pyomo.core import TransformationFactory, lnot, atleast
from pyomo.gdp import Disjunction
from pyomo.opt import SolverFactory, TerminationCondition, SolverStatus
from pyomo.util.infeasible import log_infeasible_constraints

import logging

logger = logging.getLogger(__name__)

handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
logger.addHandler(handler)
logger.setLevel(logging.INFO)
logger.propagate = False

# Create the model
model = pyo.ConcreteModel()

# topology lay
# sources r \in R
# supply tanks s \in S
# blending tanks b \in B
# demand nodes d \in D


tanks_blending = ['TK-B1', 'TK-B2']
tanks_supply = ['TK-S1', 'TK-S2']
tanks_demand = ['TK-D1', 'TK-D2']
sources = ['vessel-1', 'vessel-2']
periods = [1, 2, 3, 4, 5, 6]
specifications = ['QA', 'QB', 'QC']

bounds_flow = {('TK-S1', 'TK-B1'): [0, 1000],
               ('TK-S1', 'TK-B2'): [0, 1000],
               ('TK-S2', 'TK-B1'): [0, 1000],
               ('TK-S2', 'TK-B2'): [0, 1000],
               ('TK-B1', 'TK-D1'): [0, 1000],
               ('TK-B1', 'TK-D2'): [0, 1000],
               ('TK-B2', 'TK-D1'): [0, 1000],
               ('TK-B2', 'TK-D2'): [0, 1000],
               ('TK-S1', 'TK-D1'): [0, 1000],
               }

tanks_supply_tanks_demand = [(tki, tkd) for (tki, tkd) in bounds_flow.keys() if 'TK-S' in tki and 'TK-D' in tkd]

tanks_blend_tanks_demand = [(tki, tkd) for (tki, tkd) in bounds_flow.keys() if 'TK-B' in tki and 'TK-D' in tkd]


# Sets
def build_sets():
    model.sources_set = pyo.Set(initialize=sources, dimen=1)
    model.periods_set = pyo.Set(initialize=periods, dimen=1)
    model.specifications_set = pyo.Set(initialize=specifications, dimen=1)
    model.tanks_blending_set = pyo.Set(initialize=tanks_blending, dimen=1)
    model.tanks_supply_set = pyo.Set(initialize=tanks_supply, dimen=1)
    model.tanks_demand_set = pyo.Set(initialize=tanks_demand, dimen=1)
    model.tanks_all_set = pyo.Set(initialize=tanks_blending + tanks_supply + tanks_demand, dimen=1)
    model.tanks_to_tanks_set = pyo.Set(initialize=list(bounds_flow.keys()), dimen=2)
    model.tanks_supply_tanks_demand = pyo.Set(initialize=tanks_supply_tanks_demand, dimen=2)
    model.tanks_blend_tanks_demand = pyo.Set(initialize=tanks_blend_tanks_demand, dimen=2)


# Initial inventory for tank n In^0
# Initial values for the Specification q in tank b C0,q(b)
# Incoming supply flows enter tank s at time t Fin(s,t)
# Specification q in supply flow to tank s CIN,q (s)
# Specification q in source r C0,q (r)
# Bounds on demand flow from tanks d at time t [FD_dt_L, FD_dt_U]
# Bound on Specification q in demand tank d [C_qd_L, C_qd_U]
# Bound on Specification q in demand tank d [C_qd_L, C_qd_U]
# Bounds on inventory for tank n [I_n_L, I_b_U]
# Bounds on flow between tank n and n' [F_L_nn', F_U_nn']
# Costs for the supply flow for tank s beta_s
# Fixed costs for flow from tank n to tank n0 alfa_nn'
# variable costs for flow from tank n to tank n0 beta_nn'

# Initial inventory for tank n
initial_inventory_tanks = {'TK-B1': 20,
                           'TK-B2': 10,
                           'TK-S1': 30,
                           'TK-S2': 40,
                           'TK-D1': 50,
                           'TK-D2': 60}
# Incoming supply flows enter tank s at time t
incoming_supply = {('TK-S1', 1): 10,
                   ('TK-S2', 1): 30,
                   ('TK-S1', 2): 10,
                   ('TK-S2', 2): 30,
                   ('TK-S1', 3): 10,
                   ('TK-S2', 3): 30,
                   ('TK-S1', 4): 0,
                   ('TK-S2', 4): 0,
                   ('TK-S1', 5): 0,
                   ('TK-S2', 5): 0,
                   ('TK-S1', 6): 0,
                   ('TK-S2', 6): 0,
                   }

initial_specifications_blend = {('QA', 'TK-B1'): 0.4,
                                ('QA', 'TK-B2'): 0.4,
                                ('QB', 'TK-B1'): 0.8,
                                ('QB', 'TK-B2'): 0.8,
                                ('QC', 'TK-B1'): 1,
                                ('QC', 'TK-B2'): 1,
                                }

specifications_supply = {('QA', 'TK-S1'): 0.4,
                         ('QB', 'TK-S1'): 0.4,
                         ('QC', 'TK-S1'): 0.8,
                         ('QA', 'TK-S2'): 0.8,
                         ('QB', 'TK-S2'): 1,
                         ('QC', 'TK-S2'): 1,
                         }

specifications_source = {('QA', 'vessel-1'): 0.4,
                         ('QB', 'vessel-1'): 0.4,
                         ('QC', 'vessel-1'): 0.8,
                         ('QA', 'vessel-2'): 0.8,
                         ('QB', 'vessel-2'): 1,
                         ('QC', 'vessel-2'): 1,
                         }

bounds_demand_flow = {('TK-D1', 1): [0, 100],
                      ('TK-D1', 2): [0, 100],
                      ('TK-D1', 3): [15, 100],
                      ('TK-D1', 4): [15, 100],
                      ('TK-D1', 5): [15, 100],
                      ('TK-D1', 6): [15, 100],
                      ('TK-D2', 1): [0, 100],
                      ('TK-D2', 2): [0, 100],
                      ('TK-D2', 3): [0, 100],
                      ('TK-D2', 4): [15, 100],
                      ('TK-D2', 5): [15, 100],
                      ('TK-D2', 6): [15, 100],
                      }

bounds_specifications = {'QA': [0.0, 0.5],
                         'QB': [0.5, 1],
                         'QC': [1.5, 2],
                         }

bounds_inventory_level = {'TK-B1': [0, 15000],
                          'TK-B2': [0, 15000],
                          'TK-S1': [0, 15000],
                          'TK-S2': [0, 15000],
                          'TK-D1': [0, 15000],
                          'TK-D2': [0, 15000]}

cost_supply_flow = {'TK-S1': 0.6,
                    'TK-S2': 0.8}

fixed_cost_flow = {('TK-S1', 'TK-B1'): 3,
                   ('TK-S1', 'TK-B2'): 3,
                   ('TK-S2', 'TK-B1'): 3,
                   ('TK-S2', 'TK-B2'): 3,
                   ('TK-B1', 'TK-D1'): 3,
                   ('TK-B1', 'TK-D2'): 3,
                   ('TK-B2', 'TK-D1'): 3,
                   ('TK-B2', 'TK-D2'): 3,
                   ('TK-S1', 'TK-D1'): 3,
                   }
variable_cost_flow = {('TK-S1', 'TK-B1'): 0.1,
                      ('TK-S1', 'TK-B2'): 0.1,
                      ('TK-S2', 'TK-B1'): 0.1,
                      ('TK-S2', 'TK-B2'): 0.1,
                      ('TK-B1', 'TK-D1'): 0.1,
                      ('TK-B1', 'TK-D2'): 0.1,
                      ('TK-B2', 'TK-D1'): 0.1,
                      ('TK-B2', 'TK-D2'): 0.1,
                      ('TK-S1', 'TK-D1'): 0.1,
                      }


def build_parameters():
    # Parameters

    # Initial values for the specications q in tank b
    model.initial_specifications_blend = pyo.Param(model.specifications_set, model.tanks_blending_set,
                                                   initialize=initial_specifications_blend, within=pyo.Any)

    # (CINqs) Specication q in supply flow to tank s
    model.specifications_supply = pyo.Param(model.specifications_set, model.tanks_supply_set,
                                            initialize=specifications_supply, within=pyo.Any)

    model.cost_supply_flow = pyo.Param(model.tanks_supply_set, initialize=cost_supply_flow, within=pyo.Any)


def build_variables():
    # Decision Variables
    # (Fnn0t) Flow between tanks n and n0 at the end of time t
    model.var_flow_tanks = pyo.Var(model.tanks_to_tanks_set, model.periods_set,
                                   within=pyo.NonNegativeReals,
                                   bounds=(0, 10000))
    # (FDdt) Demand flow from tanks d at time t
    model.var_flow_demand = pyo.Var(model.tanks_demand_set, model.periods_set, within=pyo.NonNegativeReals)

    # (Xnn't) Variable that indicates the existence of flow between tanks n and n0 at the end of time t
    model.var_flag_flow = pyo.Var(model.tanks_to_tanks_set, model.periods_set, within=pyo.Binary)

    #  (Ybt.) Variable that indicates the operating mode of blending tank b at time t
    model.var_flag_blending = pyo.Var(model.tanks_blending_set, model.periods_set, within=pyo.Binary)

    model.slack_flow_demand = pyo.Var(model.tanks_demand_set, model.periods_set, within=pyo.Reals)
    model.slack_inventory_tank_supply = pyo.Var(model.tanks_supply_set, model.periods_set,
                                                within=pyo.Reals)


# Objective Function
def _expr_objective(model):
    flow_tanks_cost = sum(model.var_flow_tanks[tank_i, tank_j, period] * variable_cost_flow[tank_i, tank_j] +
                          fixed_cost_flow[tank_i, tank_j]
                          for (tank_i, tank_j) in model.tanks_to_tanks_set
                          for period in model.periods_set
                          )

    demand_flow_cost = sum(model.var_flow_demand[tank_demand, period] * 2
                           for tank_demand in model.tanks_demand_set
                           for period in model.periods_set)

    cost_blending = sum(model.var_flag_blending[tank_blend, period] * 40
                        for tank_blend in model.tanks_blending_set
                        for period in model.periods_set)

    slack_flow_cost = sum(abs(model.slack_flow_demand[tank_demand, period]) * 10e4
                          for tank_demand in model.tanks_demand_set
                          for period in model.periods_set)

    # slack_inventory_supply_cost = sum(abs(model.slack_inventory_tank_supply[tank_supply, period]) * 10e4
    #                       for tank_supply in model.tanks_supply_set
    #                       for period in model.periods_set)

    return demand_flow_cost + flow_tanks_cost + slack_flow_cost + cost_blending


def build_objective():
    model.objective = pyo.Objective(rule=_expr_objective, sense=pyo.minimize)


# (Expression) (Ist) Inventory in supply tank n at the end of time t [tank, period]
def _expr_inventory_tank_supply(model, tank_supply, period):
    if period == 1:
        return initial_inventory_tanks[tank_supply]
    else:
        all_flow_out = sum(model.var_flow_tanks[tank_supply, tank_blending, period]
                           for tank_blending in model.tanks_blending_set)
        all_flow_in = incoming_supply[tank_supply, period]
        return model.expr_inventory_tank_supply[tank_supply, period - 1] + all_flow_in - all_flow_out


# (Expression) (Idt) Inventory in demand tank n at the end of time t [tank, period]
def _expr_inventory_tank_demand(model, tank_demand, period):
    if period == 1:
        return initial_inventory_tanks[tank_demand]
    else:
        all_flow_in = sum(model.var_flow_tanks[tanks_blending, tank_demand, period]
                          for tanks_blending in model.tanks_blending_set)
        all_flow_out = model.var_flow_demand[tank_demand, period]
        return model.expr_inventory_tank_demand[tank_demand, period - 1] + all_flow_in - all_flow_out


def _expr_inventory_tank_blending(model, tank_blend, period):
    return initial_inventory_tanks[tank_blend]


def _expr_specification_in_tank(model, spec, tank_blend, period):
    return model.initial_specifications_blend[spec, tank_blend]


def build_expressions():
    # Ist [tank_supply, period]
    model.expr_inventory_tank_supply = pyo.Expression(model.tanks_supply_set, model.periods_set,
                                                      rule=_expr_inventory_tank_supply)
    # Idt [tank_supply, period]
    model.expr_inventory_tank_demand = pyo.Expression(model.tanks_demand_set, model.periods_set,
                                                      rule=_expr_inventory_tank_demand)
    model.expr_inventory_tank_blending = pyo.Expression(model.tanks_blending_set, model.periods_set,
                                                        rule=_expr_inventory_tank_blending)
    # Ibt [tank_supply, period] Inventory in blending tank n at the end of time t [tank, period]
    model.expr_specification_in_tank = pyo.Expression(model.specifications_set, model.tanks_blending_set,
                                                      model.periods_set,
                                                      rule=_expr_specification_in_tank)


# Constraints
def _constraint_inventory_tank_supply(model, tank_supply, period):
    return (bounds_inventory_level[tank_supply][0],
            model.expr_inventory_tank_supply[tank_supply, period],
            bounds_inventory_level[tank_supply][1])


def build_constraint_inventory_tank_supply():
    model.constraint_inventory_tank_supply = pyo.Constraint(model.tanks_supply_set, model.periods_set,
                                                            rule=_constraint_inventory_tank_supply)


def _constraint_inventory_tank_demand(model, tank_demand, period):
    return (bounds_inventory_level[tank_demand][0],
            model.expr_inventory_tank_demand[tank_demand, period],
            bounds_inventory_level[tank_demand][1],
            )


def build_constraint_inventory_tank_demand():
    model.constraint_inventory_tank_demand = pyo.Constraint(model.tanks_demand_set, model.periods_set,
                                                            rule=_constraint_inventory_tank_demand)


def _constraint_inventory_tank_blending(model, tank_blend, period):
    return (bounds_inventory_level[tank_blend][0],
            model.expr_inventory_tank_blending[tank_blend, period],
            bounds_inventory_level[tank_blend][1],
            )


def build_constraint_inventory_tank_blending():
    model.constraint_inventory_tank_blending = pyo.Constraint(model.tanks_blending_set, model.periods_set,
                                                              rule=_constraint_inventory_tank_blending)


def _constraint_flow_tanks(model, tank_i, tank_j, period):
    return (bounds_flow[tank_i, tank_j][0],
            model.var_flow_tanks[tank_i, tank_j, period],
            bounds_flow[tank_i, tank_j][1],
            )


def build_constraint_flow_tanks():
    model.constraint_flow_tanks = pyo.Constraint(model.tanks_to_tanks_set, model.periods_set,
                                                 rule=_constraint_flow_tanks)


def _constraint_flow_demand(model, tank_demand, period):
    return (bounds_demand_flow[tank_demand, period][0],
            model.var_flow_demand[tank_demand, period] + model.slack_flow_demand[tank_demand, period],
            bounds_demand_flow[tank_demand, period][1],
            )


def build_constraint_flow_demand():
    model.constraint_flow_demand = pyo.Constraint(model.tanks_demand_set, model.periods_set,
                                                  rule=_constraint_flow_demand)


def _constraint_spec_concentration(model, spec, tank_blend, period):
    return (bounds_specifications[spec][0],
            model.expr_specification_in_tank[spec, tank_blend, period],
            bounds_specifications[spec][1],
            )


def build_constraint_spec_blend():
    model.constraint_spec_concentration = pyo.Constraint(model.specifications_set,
                                                         model.tanks_blending_set,
                                                         model.periods_set,
                                                         rule=_constraint_spec_concentration)


def _exp_disjunctions_1(model, tank_i, tank_j, period):
    left_side_conditions = [model.var_flag_flow[tank_i, tank_j, period] == 1,
                            model.var_flow_tanks[tank_i, tank_j, period] <= bounds_flow[tank_i, tank_j][1],
                            model.var_flow_tanks[tank_i, tank_j, period] >= bounds_flow[tank_i, tank_j][0]]
    right_side_conditions = [model.var_flag_flow[tank_i, tank_j, period] == 0,
                             model.var_flow_tanks[tank_i, tank_j, period] == 0]
    return [left_side_conditions,
            right_side_conditions]


def build_disjunction_logic_flow():
    model.disjunctions_1 = Disjunction(model.tanks_to_tanks_set, model.periods_set,
                                       rule=_exp_disjunctions_1)


# Disjunction flow between nodes (I)
# If the flow between nodes exists (var_flag_flow = 1), then upper and lower bounds on flow and concentration are enforced.
# If the flow does not exist, then the flow is zero and no concentration constraints are enforced.
def build_disjunction_flow_between_nodes():
    model.constraint_flow_dis_2aa = pyo.ConstraintList()
    model.constraint_flow_dis_2ab = pyo.ConstraintList()
    model.constraint_flow_dis_2ac = pyo.ConstraintList()
    model.constraint_flow_dis_2ba = pyo.ConstraintList()
    model.constraint_flow_dis_2bb = pyo.ConstraintList()
    for tank_supply, tank_demand in model.tanks_supply_tanks_demand:
        for period in model.periods_set:
            meet_upper_bound = [model.specifications_supply[spec, tank_supply] <=
                                bounds_specifications[spec][1]
                                for spec in model.specifications_set]
            meet_lower_bound = [model.specifications_supply[spec, tank_supply] >=
                                bounds_specifications[spec][0]
                                for spec in model.specifications_set]
            if all(meet_upper_bound) and all(meet_lower_bound):

                model.constraint_flow_dis_2aa.add(model.var_flag_flow[tank_supply, tank_demand, period] == 1)
                model.constraint_flow_dis_2ab.add(model.var_flow_tanks[tank_supply, tank_demand, period] <=
                                                  bounds_flow[tank_supply, tank_demand][1])
                model.constraint_flow_dis_2ac.add(model.var_flow_tanks[tank_supply, tank_demand, period] >=
                                                  bounds_flow[tank_supply, tank_demand][0])
            else:
                model.constraint_flow_dis_2ba.add(model.var_flag_flow[tank_supply, tank_demand, period] == 0)
                model.constraint_flow_dis_2bb.add(model.var_flow_tanks[tank_supply, tank_demand, period] == 0)


def _exp_disjunctions_3(model, tank_blend, tank_demand, period):
    """
    Disjunction flow between nodes (II)
    this disjunction enforces that there can only exist flow
    between supply and demand when the supply specs lie within the demand bounds
    """
    if period > 1:
        left_side_conditions = [model.var_flag_flow[tank_blend, tank_demand, period] == 1,
                                model.var_flow_tanks[tank_blend, tank_demand, period] <=
                                bounds_flow[tank_blend, tank_demand][1],
                                model.var_flow_tanks[tank_blend, tank_demand, period] >=
                                bounds_flow[tank_blend, tank_demand][0]
                                ]
        for spec in model.specifications_set:
            left_side_conditions.append(model.expr_specification_in_tank[spec, tank_blend, period - 1] <=
                                        bounds_specifications[spec][1])
            left_side_conditions.append(model.expr_specification_in_tank[spec, tank_blend, period - 1] >=
                                        bounds_specifications[spec][0])

        right_side_conditions = [model.var_flag_flow[tank_blend, tank_demand, period] == 0,
                                 model.var_flow_tanks[tank_blend, tank_demand, period] == 0,
                                 ]
        return [left_side_conditions,
                right_side_conditions]
    else:
        return Disjunction.Skip


def _exp_disjunction_logic(model, tank_i, tank_j, period):
    if tank_j in model.tanks_blending_set:
        left_side_conditions = [
            model.var_flag_flow[tank_i, tank_j, period] + model.var_flag_blending[tank_j, period] == 0,
            model.var_flag_flow[tank_i, tank_j, period] <= 1,
            model.var_flag_blending[tank_i, tank_j, period] <= 1,
        ]
        right_side_conditions = \
            model.var_flag_flow[tank_i, tank_j, period] + model.var_flag_blending[tank_j, period] == 2

        return [left_side_conditions,
                right_side_conditions]
    else:
        return Disjunction.Skip


def _constraint_logic_binaries_1(model, tank_i, tank_j, period):
    if tank_j in model.tanks_blending_set:
        return model.var_flag_flow[tank_i, tank_j, period] + model.var_flag_blending[tank_j, period] == 2
    else:
        return pyo.Constraint.Skip


def _constraint_logic_binaries_2(model, tank_i, tank_j, period):
    if tank_j in model.tanks_blending_set:
        return model.var_flag_flow[tank_i, tank_j, period] == model.var_flag_blending[tank_j, period]
    else:
        return pyo.Constraint.Skip


def build_logical_constraint_flow_and_blending():
    # model.disjunction_flow_and_blending = pyo.Constraint(model.tanks_to_tanks_set, model.periods_set,
    #                                                      rule=_exp_disjunction_logic)
    # model.constraint_logic_binaries_1 = pyo.Constraint(model.tanks_to_tanks_set, model.periods_set,
    #                                                    rule=_constraint_logic_binaries_1)
    model.constraint_logic_binaries_2 = pyo.Constraint(model.tanks_to_tanks_set, model.periods_set,
                                                       rule=_constraint_logic_binaries_2)


# def _expr_logical_constraint(model, tank_i, tank_j, period):
#     if tank_j in model.tanks_blending_set:
#         return model.var_flag_flow[tank_i, tank_j, period].implies(model.var_flag_blending[tank_j, period])
#     else:
#         return pyo.LogicalConstraint.Skip
#
#
# def build_logical_constraint_flow_and_blending():
#     model.logical_constraint = pyo.LogicalConstraint(model.tanks_to_tanks_set, model.periods_set,
#                                                      rule=_expr_logical_constraint)


def build_disjunction_flow_between_source_and_demand():
    model.disjunctions_3 = Disjunction(model.tanks_blend_tanks_demand,
                                       model.periods_set,
                                       rule=_exp_disjunctions_3)


def _exp_disjunctions_4(model, tank_blend, period):
    """
    Disjunction operation of the blending tanks.
    Since there cannot be simultaneous input/output streams to blending tanks, they can be either charging or discharging
    but not both. The total mass balance of the inventory is calculated if a tank is either charging
    or discharging.
    """
    if period > 1:
        # Blend tank charge
        left_side_conditions = [model.var_flag_blending[tank_blend, period] == 1,
                                model.expr_inventory_tank_blending[tank_blend, period] == (
                                        model.expr_inventory_tank_blending[tank_blend, period - 1] +
                                        sum(model.var_flow_tanks[tank_supply, tank_blend, period]
                                            for tank_supply in model.tanks_supply_set)
                                ),
                                ]

        for spec in model.specifications_set:
            expr_to_append = (model.expr_inventory_tank_blending[tank_blend, period] *
                              model.expr_specification_in_tank[spec, tank_blend, period] == (
                                      model.expr_inventory_tank_blending[tank_blend, period - 1] *
                                      model.expr_specification_in_tank[spec, tank_blend, period - 1] +
                                      sum(model.var_flow_tanks[tank_supply, tank_blend, period] *
                                          model.specifications_supply[spec, tank_supply]
                                          for tank_supply in model.tanks_supply_set)
                              )
                              )
            left_side_conditions.append(expr_to_append)

        # Blend tank discharge
        right_side_conditions = [model.var_flag_blending[tank_blend, period] == 0,
                                 model.expr_inventory_tank_blending[tank_blend, period] == (
                                         model.expr_inventory_tank_blending[tank_blend, period - 1] -
                                         sum(model.var_flow_tanks[tank_blend, tank_demand, period]
                                             for tank_demand in model.tanks_demand_set)
                                 ),
                                 ]
        for spec in model.specifications_set:
            expr_to_append = model.expr_specification_in_tank[spec, tank_blend, period] == \
                             model.expr_specification_in_tank[spec, tank_blend, period - 1]
            right_side_conditions.append(expr_to_append)

        return [left_side_conditions, right_side_conditions]
    else:
        return Disjunction.Skip


def build_disjunction_logic_blending():
    model.disjunctions_4 = Disjunction(model.tanks_blending_set, model.periods_set,
                                       rule=_exp_disjunctions_4)


def apply_reformulation(mode: str = 'gdp.hull'):
    """
    'gdp.cuttingplane' --> Hybrid BM/HR Reformulation
    'gdp.hull' --> Hull Reformulation (HR)
    'gdp.bigm' --> Big-M (BM) Reformulation
    """
    TransformationFactory(mode).apply_to(model)


def solve(solver: str = 'scip'):
    solver = 'scip'
    # solver = 'cbc'
    # Solve the model
    optimizer = SolverFactory(solver)

    if solver == 'scip':
        optimizer.options['limits/time'] = 150
        optimizer.options['limits/gap'] = 0.005  # 0.5 %
    if solver == 'cbc':
        optimizer.options['integertolerance'] = 1e-4
        optimizer.options['ratioGAP'] = 0.0005  # 0.05 %
        optimizer.options['timeMode'] = 'elapsed'
        optimizer.options['seconds'] = 300

    results = optimizer.solve(model, tee=True)
    return results


def has_solution(results, model):
    status = results.solver.status
    condition = results.solver.termination_condition

    infeasible = condition == pyo.TerminationCondition.infeasible
    unbounded = condition == pyo.TerminationCondition.unbounded
    unknown = condition == pyo.TerminationCondition.unknown
    infeasible_or_unbounded = condition == pyo.TerminationCondition.infeasibleOrUnbounded
    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    optimal = condition == pyo.TerminationCondition.optimal
    feasible = condition == pyo.TerminationCondition.feasible

    other_condition = condition == pyo.TerminationCondition.other
    solution = (status == pyo.SolverStatus.ok and optimal) or \
               (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit) or \
               (status == pyo.SolverStatus.ok and other_condition)  # GAP limit in SCIP

    if feasible:
        print('Feasible Solution founded !')

    if not solution:
        print(results)
        print(
            f"Something is wrong -> status {status} "
            f"and condition {condition}"
        )
        if infeasible or infeasible_or_unbounded or unbounded or unknown:
            log_infeasible_constraints(
                model,
                logger=logger,
                log_expression=True,
                log_variables=True,
            )
        raise Exception('Error: Problem is infeasible or unbounded')

    return solution


def build_solution(results, model):
    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())

    print('\n')
    objective_value = pyo.value(model.objective)
    print('Objective value: {}'.format(objective_value))

    rows = []
    print('\n')
    for ((tank_i, tank_j, period), var) in model.var_flow_tanks.items():
        rows.append((tank_i, tank_j, period, pyo.value(var)))
        print('var_flow_tanks({},{},{}): {}'.format(tank_i, tank_j, period, pyo.value(var)))
    df_var_flow = DataFrame(data=rows, columns=['TankI', 'TankJ', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank_i, period), var) in model.var_flow_demand.items():
        rows.append((tank_i, period, pyo.value(var)))
        print('var_flow_demand({},{}): {}'.format(tank_i, period, pyo.value(var)))
    df_var_flow_demand = DataFrame(data=rows, columns=['Tank', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank_i, tnak_j, period), var) in model.var_flag_flow.items():
        rows.append((tank_i, tank_j, period, pyo.value(var)))
        print('var_flag_flow({},{},{}): {}'.format(tank_i, tank_j, period, pyo.value(var)))
    df_var_flag_flow = DataFrame(data=rows, columns=['TankI', 'TankJ', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank, period), var) in model.var_flag_blending.items():
        rows.append((tank, period, pyo.value(var)))
        print('var_flag_blending({},{}): {}'.format(tank, period, pyo.value(var)))
    df_var_flag_blending = DataFrame(data=rows, columns=['Tank', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank, period), var) in model.expr_inventory_tank_supply.items():
        rows.append((tank, period, pyo.value(var)))
        print('expr_inventory_tank_supply({},{}): {}'.format(tank, period, pyo.value(var)))
    df_exp_inventory_tank_supply = DataFrame(data=rows, columns=['Tank', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank, period), var) in model.expr_inventory_tank_blending.items():
        rows.append((tank, period, pyo.value(var)))
        print('expr_inventory_tank_blending({},{}): {}'.format(tank, period, pyo.value(var)))
    df_exp_inventory_tank_blending = DataFrame(data=rows, columns=['Tank', 'Period', 'Value'])

    rows = []
    print('\n')
    for ((tank, period), var) in model.expr_inventory_tank_demand.items():
        rows.append((tank, period, pyo.value(var)))
        print('expr_inventory_tank_demand({},{}): {}'.format(tank, period, pyo.value(var)))
    df_exp_inventory_tank_demand = DataFrame(data=rows, columns=['Tank', 'Period', 'Value'])

    df_inventory = concat([df_exp_inventory_tank_supply, df_exp_inventory_tank_blending])
    df_inventory = concat([df_inventory, df_exp_inventory_tank_demand])

    rows = []
    print('\n')
    for ((spec, tank_blend, period), var) in model.expr_specification_in_tank.items():
        rows.append((spec, tank_blend, period, pyo.value(var)))
        print('expr_specification_in_tank({},{},{}): {}'.format(spec, tank_blend, period, pyo.value(var)))
    df_exp_specification_in_tank = DataFrame(data=rows, columns=['Spec', 'Tank', 'Period', 'Value'])
    df_exp_specification_in_tank['Spec-Tank'] = (
        df_exp_specification_in_tank.apply(lambda x: x['Spec'] + '-' + x['Tank'], axis=1))

    print('\n')
    for ((tank_demand, period), var) in model.slack_flow_demand.items():
        print('slack_flow_demand({},{}): {}'.format(tank_demand, period, pyo.value(var)))

    # print('\n')
    # for ((tank_supply, period), var) in model.slack_inventory_tank_supply.items():
    #     print('slack_inventory_tank_supply({},{}): {}'.format(tank_supply, period, pyo.value(var)))

    return df_exp_specification_in_tank, df_inventory


def plot_solution(df_exp_specification_in_tank,
                  df_inventory):
    fig = make_subplots(rows=3, cols=1)

    for combi in df_exp_specification_in_tank['Spec-Tank'].unique():
        df_aux = df_exp_specification_in_tank[df_exp_specification_in_tank['Spec-Tank'] == combi]
        fig.add_trace(go.Scatter(
            x=df_aux['Period'],
            y=df_aux['Value'],
            name=combi,
        ), row=1, col=1)

    for tank in df_inventory['Tank'].unique():
        df_aux = df_inventory[df_inventory['Tank'] == tank]
        fig.add_trace(go.Scatter(
            x=df_aux['Period'],
            y=df_aux['Value'],
            name='Volume-' + tank,
        ), row=2, col=1)

    fig.update_layout(
        # legend_title="Price Ref stock",
        font=dict(
            family="Courier New, monospace",
            size=14,
            color="RebeccaPurple"
        )
    )
    fig.update_layout(yaxis_tickfont_size=14)
    fig.update_yaxes(tickangle=0)
    fig.update_traces(opacity=0.75)
    fig.update_layout(height=500, width=900)

    # fig = px.line(df_exp_inventory_tank_supply, x="Period", y="Value", color="Tank", title='Inventory tank supply')
    # fig.show()
    #
    # fig = px.line(df_exp_specification_in_tank, x="Period", y="Value", color="Tank", line_group="Spec",
    #               title='Spec in tank')
    fig.show()


def main():
    build_sets()
    build_parameters()
    build_variables()
    build_expressions()
    build_objective()
    build_constraint_inventory_tank_supply()
    build_constraint_inventory_tank_demand()
    build_constraint_inventory_tank_blending()
    build_constraint_flow_tanks()
    build_constraint_flow_demand()
    build_logical_constraint_flow_and_blending()
    # build_constraint_spec_blend()
    build_disjunction_logic_flow()
    # build_disjunction_flow_between_nodes()
    build_disjunction_flow_between_source_and_demand()
    build_disjunction_logic_blending()
    apply_reformulation(mode='gdp.bigm')

    results = solve(solver='scip')

    solution = has_solution(results, model)

    if solution:
        df_exp_specification_in_tank, df_inventory = build_solution(results, model)
        plot_solution(df_exp_specification_in_tank, df_inventory)
