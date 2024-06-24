# Import Pyomo and the required components
import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# Create a concrete Pyomo model
model = pyo.ConcreteModel()

# Sets
vessels = ['Vessel1', 'Vessel2']
demands = ['VLSFO', 'LFSO']
periods = [1, 2, 3, 4, 5, 6]

tanks = ['TK1', 'TK2', 'TK3', 'TK4', 'TK5', 'TK6', 'TK7', 'TK8']
tanks_comp = ['TK1', 'TK2', 'TK3']
tanks_blend = ['TK4', 'TK5', ]
tanks_prod = ['TK6', 'TK7', 'TK8']

# Parameters
capacity_vessel = {(1, 'Vessel1'): 1500,
                   (1, 'Vessel2'): 1500,
                   (2, 'Vessel1'): 1500,
                   (2, 'Vessel2'): 1500,
                   (3, 'Vessel1'): 1500,
                   (3, 'Vessel2'): 1500,
                   (4, 'Vessel1'): 1500,
                   (4, 'Vessel2'): 1500,
                   (5, 'Vessel1'): 1400,
                   (5, 'Vessel2'): 1400,
                   (6, 'Vessel1'): 1500,
                   (6, 'Vessel2'): 1500,
                   }

# Fuel demand of each tank
tank_demand = {'TankA': 3000,
               'TankB': 5000,
               'TankC': 7000}

# Cost to transport fuel from each vessel to each tank
transport_cost = {
    ('Vessel1', 'TankA'): 10,
    ('Vessel1', 'TankB'): 12,
    ('Vessel1', 'TankC'): 15,
    ('Vessel2', 'TankA'): 8,
    ('Vessel2', 'TankB'): 10,
    ('Vessel2', 'TankC'): 14,
    ('Vessel3', 'TankA'): 6,
    ('Vessel3', 'TankB'): 9,
    ('Vessel3', 'TankC'): 12
}

# Variables
model.flow_discharge = pyo.Var(vessels, periods, tanks_comp, domain=pyo.NonNegativeReals)
model.flow_to_blend = pyo.Var(tanks_comp, periods, tanks_blend, domain=pyo.NonNegativeReals)
model.flow_to_prod = pyo.Var(tanks_blend, periods, tanks_prod, domain=pyo.NonNegativeReals)
model.flow_to_supply = pyo.Var(tanks_prod, periods, demands, domain=pyo.NonNegativeReals)

model.flag_discharge = pyo.Var(vessels, periods, tanks_comp, domain=pyo.Binary)
model.flag_blend = pyo.Var(tanks_comp, periods, tanks_blend, domain=pyo.Binary)
model.flag_to_prod = pyo.Var(tanks_blend, periods, tanks_prod, domain=pyo.Binary)
model.flag_to_supply = pyo.Var(tanks_prod, periods, demands, domain=pyo.Binary)

F_max = 5000
F_min = 500

param_volume_tank_comp_init = 200

V_min = 100
V_max = 30000


# Amount of fuel to be transported from each vessel to each tank

def _cost_discharge(model):
    return sum(model.flow_discharge[vessel, period, tanks_comp]
               for tanks_comp in tanks_comp
               for vessel in vessels
               for period in periods) * 1e4


model.cost_discharge = pyo.Expression(rule=_cost_discharge)


def _cost_blend(model):
    return sum(model.flow_to_blend[tank_comp, period, tank_blend]
               for tank_comp in tanks_comp
               for period in periods
               for tank_blend in tanks_blend) * 1e4


model.cost_blend = pyo.Expression(rule=_cost_blend)


def _cost_product(model):
    return sum(model.flow_to_prod[tank_b, period, tank_prod]
               for tank_b in tanks_blend
               for period in periods
               for tank_prod in tanks_prod) * 1e4


model.cost_product = pyo.Expression(rule=_cost_product)


def _cost_supply(model):
    return sum(model.flow_to_supply[tank_prod, period, demand]
               for tank_prod in tanks_prod
               for period in periods
               for demand in demands) * 1e4


model.cost_supply = pyo.Expression(rule=_cost_supply)

objective_expression = model.cost_discharge + model.cost_blend + model.cost_product + model.cost_supply

# Objective function: Minimize the total transportation cost
model.objective = pyo.Objective(
    expr=objective_expression,
    sense=pyo.minimize)


def _exp_volume_tank_comp(model, period, tank_comp):
    if period == 1:
        return param_volume_tank_comp_init
    else:
        total_output = sum(model.flow_to_blend[tank_comp, period, tank_blend] for tank_blend in tanks_blend)
        total_input = sum(model.flow_discharge[vessel, period, tank_comp] for vessel in vessels)
        return model.volume_tanks_comp[period - 1, tank_comp] - total_output + total_input


model.volume_tanks_comp = pyo.Expression(periods, tanks_comp, rule=_exp_volume_tank_comp)


# Constraints
def _constraint_vessel_flow_discharge_capacity_upper(model, period, tank, vessel):
    return model.flow_discharge[vessel, period, tank] <= F_max * \
        model.flag_discharge[vessel, period, tank]


model.constraint_vessel_flow_discharge_capacity_upper = (
    pyo.Constraint(periods, tanks_comp, vessels, rule=_constraint_vessel_flow_discharge_capacity_upper))


def _constraint_vessel_flow_discharge_capacity_lower(model, period, tank, vessel):
    return F_max * model.flag_discharge[vessel, period, tank] <= model.flow_discharge[vessel, period, tank]


model.constraint_vessel_flow_discharge_capacity_lower = (
    pyo.Constraint(periods, tanks_comp, vessels, rule=_constraint_vessel_flow_discharge_capacity_lower))


def _constraint_vessel_discharge_capacity(model, period, tank, vessel):
    return model.flag_discharge[vessel, period, tank] <= capacity_vessel[(period, vessel)]


model.constraint_vessel_discharge_capacity = pyo.Constraint(periods, tanks_comp, vessels,
                                                            rule=_constraint_vessel_discharge_capacity)


def _constraint_volume_tank_comp_upper(model, tank_comp, period):
    return model.volume_tanks_comp[period, tank_comp] <= V_max


model.constraint_volume_tank_comp_upper = pyo.Constraint(tanks_comp, periods,
                                                         rule=_constraint_volume_tank_comp_upper)


def _constraint_volume_tank_comp_lower(model, tank_comp, period):
    return V_min <= model.volume_tanks_comp[period, tank_comp]


model.constraint_volume_tank_comp_lower = pyo.Constraint(tanks_comp, periods,
                                                         rule=_constraint_volume_tank_comp_lower)

# Solve the optimization problem
opt = SolverFactory('scip')

results = opt.solve(model, tee=True)  # tee=True for details

# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())

    for period in periods:
        for tank_comp in tanks_comp:
            value_ = pyo.value(model.volume_tanks_comp[period, tank_comp])
            print('Volume tank comp ({},{}):{}'.format(period, tank_comp, value_))

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
