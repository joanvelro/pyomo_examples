from pyomo.environ import *
from pyomo.gdp import Disjunction

# Create the model
model = ConcreteModel()

# topology lay
# sources r \in R
# supply tanks s \in S
# blending tanks b \in B
# demand nodes d \in D


tanks_blending = ['TK-B1', 'TK-B2']
tanks_supply = ['TK-S1', 'TK-S2']
tanks_demand = ['TK-D1', 'TK-D2']
sources = ['vessel-1', 'vessel-2']
periods = [1, 2, 3]
specifications = ['Q1', 'Q2', 'Q3']

bounds_flow = {('TK-S1', 'TK-B1'): [0, 100],
               ('TK-S1', 'TK-B2'): [0, 100],
               ('TK-S2', 'TK-B1'): [0, 100],
               ('TK-S2', 'TK-B2'): [0, 100],
               ('TK-B1', 'TK-D1'): [0, 100],
               ('TK-B1', 'TK-D2'): [0, 100],
               ('TK-B2', 'TK-D1'): [0, 100],
               ('TK-B2', 'TK-D2'): [0, 100],
               }

# Sets
model.sources_set = Set(initialize=sources, dimen=1)
model.periods_set = Set(initialize=periods, dimen=1)
model.specifications_set = Set(initialize=specifications, dimen=1)
model.tanks_blending_set = Set(initialize=tanks_blending, dimen=1)
model.tanks_supply_set = Set(initialize=tanks_supply, dimen=1)
model.tanks_demand_set = Set(initialize=tanks_demand, dimen=1)
model.all_tanks_set = Set(initialize=tanks_blending + tanks_supply + tanks_demand, dimen=1)
model.tanks_to_tanks_set = Set(initialize=list(bounds_flow.keys()), dimen=2)

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

initial_inventory_tanks = {'TK-B1': 20,
                           'TK-B2': 10,
                           'TK-S1': 30,
                           'TK-S2': 40,
                           'TK-D1': 50,
                           'TK-D2': 60}

initial_specifications = {('Q1', 'TK-B1'): 0.7,
                          ('Q1', 'TK-B2'): 0.7,
                          ('Q2', 'TK-B1'): 0.7,
                          ('Q2', 'TK-B2'): 0.7,
                          ('Q3', 'TK-B1'): 0.7,
                          ('Q3', 'TK-B2'): 0.7
                          }

incoming_supply = {('TK-S1', 1): 50,
                   ('TK-S2', 1): 50,
                   ('TK-S1', 2): 50,
                   ('TK-S2', 2): 50,
                   ('TK-S1', 3): 50,
                   ('TK-S2', 3): 50,
                   }

specifications_supply = {('Q1', 'TK-S1'): 0.97,
                         ('Q2', 'TK-S1'): 0.97,
                         ('Q3', 'TK-S1'): 0.97,
                         ('Q1', 'TK-S2'): 0.97,
                         ('Q2', 'TK-S2'): 0.97,
                         ('Q3', 'TK-S2'): 0.97,
                         }

specifications_source = {('Q1', 'vessel-1'): 0.97,
                         ('Q2', 'vessel-1'): 0.97,
                         ('Q3', 'vessel-1'): 0.97,
                         ('Q1', 'vessel-2'): 0.97,
                         ('Q2', 'vessel-2'): 0.97,
                         ('Q3', 'vessel-2'): 0.97,
                         }

bounds_demand_flow = {('TK-D1', 1): [0, 300],
                      ('TK-D1', 2): [0, 300],
                      ('TK-D1', 3): [0, 300],
                      ('TK-D2', 1): [0, 300],
                      ('TK-D2', 2): [0, 300],
                      ('TK-D2', 3): [0, 300],
                      }

bounds_specifications_demand_tank = {('Q1', 'TK-D1'): [0.98, 1.05],
                                     ('Q1', 'TK-D2'): [0.98, 1.05],
                                     ('Q2', 'TK-D1'): [0.98, 1.05],
                                     ('Q2', 'TK-D2'): [0.98, 1.05],
                                     ('Q3', 'TK-D1'): [0.98, 1.05],
                                     ('Q3', 'TK-D2'): [0.98, 1.05],
                                     }

bounds_inventory_level = {'TK-B1': [0, 500],
                          'TK-B2': [0, 500],
                          'TK-S1': [0, 500],
                          'TK-S2': [0, 500],
                          'TK-D1': [0, 500],
                          'TK-D2': [0, 500]}

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
                   }
variable_cost_flow = {('TK-S1', 'TK-B1'): 0.1,
                      ('TK-S1', 'TK-B2'): 0.1,
                      ('TK-S2', 'TK-B1'): 0.1,
                      ('TK-S2', 'TK-B2'): 0.1,
                      ('TK-B1', 'TK-D1'): 0.1,
                      ('TK-B1', 'TK-D2'): 0.1,
                      ('TK-B2', 'TK-D1'): 0.1,
                      ('TK-B2', 'TK-D2'): 0.1,
                      }

# Parameters
model.initial_inventory_tanks = Param(model.all_tanks_set,
                                      initialize=initial_inventory_tanks, domain=Any)

model.initial_specifications = Param(model.specifications_set, model.tanks_blending_set,
                                     initialize=initial_specifications, domain=Any)

model.incoming_supply = Param(model.tanks_supply_set, model.periods_set,
                              initialize=incoming_supply, domain=Any)

model.specifications_supply = Param(model.specifications_set, model.tanks_supply_set,
                                    initialize=specifications_supply, domain=Any)

model.specifications_source = Param(model.specifications_set, model.sources_set,
                                    initialize=specifications_source, domain=Any)

model.bounds_demand_flow = Param(model.tanks_demand_set, model.periods_set,
                                 initialize=bounds_demand_flow, domain=Any)

model.bounds_specifications_demand_tank = Param(model.specifications_set, model.tanks_demand_set,
                                                initialize=bounds_specifications_demand_tank, domain=Any)

model.bounds_inventory_level = Param(model.all_tanks_set, initialize=bounds_inventory_level, domain=Any)

model.bounds_flow = Param(model.tanks_to_tanks_set, initialize=bounds_flow, domain=Any)

model.cost_supply_flow = Param(model.tanks_supply_set, initialize=cost_supply_flow, domain=Any)

model.fixed_cost_flow = Param(model.tanks_to_tanks_set, initialize=fixed_cost_flow, domain=Any)

model.variable_cost_flow = Param(model.tanks_to_tanks_set, initialize=variable_cost_flow, domain=Any)

# Decision Variables
model.var_flow_tanks = Var(model.tanks_to_tanks_set, model.periods_set,
                           within=NonNegativeReals,
                           bounds=(0, 500))
model.var_demand_flow = Var(model.tanks_demand_set, model.periods_set, within=NonNegativeReals)
model.var_inventory_tank = Var(model.all_tanks_set, model.periods_set, within=NonNegativeReals)
model.var_specification_in_tank = Var(model.specifications_set, model.tanks_blending_set, model.periods_set,
                                      within=NonNegativeReals)
model.var_specification_flow = Var(model.specifications_set, model.tanks_blending_set, model.periods_set,
                                   within=NonNegativeReals)
model.var_flow_source = Var(model.sources_set, model.all_tanks_set, model.periods_set,
                            within=NonNegativeReals)
model.var_inventory_source = Var(model.sources_set, model.tanks_blending_set, model.periods_set,
                                 within=NonNegativeReals)

model.var_flag_flow = Var(model.tanks_to_tanks_set, model.periods_set,
                          within=Binary)

model.var_flag_blending = Var(model.tanks_blending_set, model.periods_set,
                              within=Binary)

model.var_flag_blending_bool = Var(model.tanks_blending_set, model.periods_set,
                                   within=Boolean)


# Objective Function
def obj_rule(model):
    return sum(model.var_flow_tanks[tank_i, tank_j, period] * model.variable_cost_flow[tank_i, tank_j] +
               model.fixed_cost_flow[tank_i, tank_j]
               for (tank_i, tank_j) in model.tanks_to_tanks_set
               for period in model.periods_set
               )


model.obj = Objective(rule=obj_rule, sense=minimize)


def rule_constraint_disjunctions(model, tank_i, tank_j, period):
    left_side_conditions = [model.var_flag_flow[tank_i, tank_j, period] == 1,
                            model.var_flow_tanks[tank_i, tank_j, period] <= model.bounds_flow[tank_i, tank_j][1],
                            model.var_flow_tanks[tank_i, tank_j, period] >= model.bounds_flow[tank_i, tank_j][0]]
    right_side_conditions = [model.var_flag_flow[tank_i, tank_j, period] == 0,
                             model.var_flow_tanks[tank_i, tank_j, period] == 0]
    return [left_side_conditions,
            right_side_conditions]


model.disjunctions = Disjunction(model.tanks_to_tanks_set, model.periods_set,
                                 rule=rule_constraint_disjunctions)

# Constraints
# def availability_constraint_rule(model, c, t):
#     return model.var_quantity[c, t] <= model.param_availability[c, t]
#
#
# model.availability_constraint = Constraint(model.Components, model.Periods, rule=availability_constraint_rule)


TransformationFactory('gdp.hull').apply_to(model)

# Solve the model
optimizer = SolverFactory('cbc')  # Replace <solver_name> with the name of the solver you want to use

results = optimizer.solve(model)

if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print('--- Optimal solution ---')

    print(results.Problem())
    # print(results.Solution())
    print(results.Solver())

elif (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.feasible):
    print('--- Feasible solution ---')

    print(results.Problem())
    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible or \
        results.solver.termination_condition == TerminationCondition.infeasibleOrUnbounded:
    print('(!) Infeasible or unbounded solution')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
