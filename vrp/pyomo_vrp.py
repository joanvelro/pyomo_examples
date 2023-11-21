import logging
import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
from pyomo.util.infeasible import log_infeasible_constraints

logger = logging.getLogger(__name__)
level: int = logging.DEBUG
logger.setLevel(level)

# Model
model = pyo.ConcreteModel()

# Sets
nodes = ['Depot', 'Customer1', 'Customer2', 'Customer3']  # Set of nodes (depot and customers)

model.set_nodes = pyo.Set(initialize=nodes)

vehicles = ['Vehicle1', 'Vehicle2']  # Set of vehicles

model.set_vehicles = pyo.Set(initialize=vehicles)

# Parameters
demand = {'Depot': 0,
          'Customer1': 10,
          'Customer2': 15,
          'Customer3': 8}  # Demand at each customer node
time_windows = {'Customer1': (8, 12),
                'Customer2': (9, 13),
                'Customer3': (10, 14)}
# Time windows for each customer node
vehicle_capacity = {'Vehicle1': 60,
                    'Vehicle2': 65}  # Capacity of each vehicle
service_time = {'Customer1': 2,
                'Customer2': 3,
                'Customer3': 2}  # Service time at each customer node
travel_time = {'Depot': {'Customer1': 10, 'Customer2': 15, 'Customer3': 20},
               'Customer1': {'Depot': 10, 'Customer2': 5, 'Customer3': 15},
               'Customer2': {'Depot': 15, 'Customer1': 5, 'Customer3': 10},
               'Customer3': {'Depot': 20, 'Customer1': 15, 'Customer2': 10}}  # Travel time between nodes

# Decision Variables
domain_x = [tuple((str(node1), str(node2), str(vehicle))) for node1 in nodes for node2 in nodes for vehicle in vehicles]
model.x = pyo.Var(domain_x, within=pyo.Binary)

# Binary variable indicating if a vehicle travels from one node to another
model.u = pyo.Var(nodes, within=pyo.NonNegativeReals)  # Variable representing the time at which a node is visited

# Constraints
model.flow_constraint = pyo.ConstraintList()
for j in nodes[1:]:
    model.flow_constraint.add(
        sum(model.x[i, j, v] for i in nodes for v in vehicles) == 1)  # Each customer node is visited exactly once


def obj_rule(model):
    return sum(travel_time[node_i][node_j] * model.x[node_i, node_j, vehicle]
               for (node_i, node_j, vehicle) in domain_x if node_i != node_j)


# Objective Function
model.cost = pyo.Objective(rule=obj_rule, sense=pyo.minimize)


def constraint_capacity_rule(model, vehicle, node_j):
    total_quantity = sum(demand[node_j] * model.x[node_i, node_j, vehicle] for node_i in model.set_nodes
                         if node_i != node_j and (node_i, node_j, vehicle) in domain_x)
    return total_quantity <= vehicle_capacity[vehicle]


model.constraint_capacity = pyo.Constraint(model.set_vehicles, model.set_nodes, rule=constraint_capacity_rule)

def constraint_depot_rule(model, vehicle):
    return sum(model.x['Depot', node_j, vehicle] for node_j in model.set_nodes if node_j != 'Depot') == 1

model.constraint_depot_rule = pyo.Constraint(model.set_vehicles, rule=constraint_depot_rule)

model.time_window_constraint = pyo.ConstraintList()
for v in vehicles:
    for j in nodes[1:]:
        model.time_window_constraint.add(model.u[j] >= time_windows[j][0])  # Lower time window constraint
        model.time_window_constraint.add(model.u[j] <= time_windows[j][1])  # Upper time window constraint
        for i in nodes[1:]:
            if i != j:
                model.time_window_constraint.add(model.u[j] >= model.u[i] + service_time[i] +
                                                 travel_time[i][j] - time_windows[j][1])

# Solve the model
solver = SolverFactory('cbc')
results = solver.solve(model, tee=True)

status = results.solver.status
condition = results.solver.termination_condition

infeasible = condition == pyo.TerminationCondition.infeasible
unbounded = condition == pyo.TerminationCondition.unbounded
unknown = condition == pyo.TerminationCondition.unknown
infeasible_or_unbounded = condition == pyo.TerminationCondition.infeasibleOrUnbounded
max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
optimal = condition == pyo.TerminationCondition.optimal
feasible = condition == pyo.TerminationCondition.feasible
# feasible_or_optimal = (
#        condition == pyo.TerminationCondition.optimal
#        or condition == pyo.TerminationCondition.feasible
# )
other_condition = condition == pyo.TerminationCondition.other
solution = (status == pyo.SolverStatus.ok and optimal) or \
           (status == pyo.SolverStatus.ok and feasible) or \
           (status == pyo.SolverStatus.aborted and max_time_limit) or \
           (status == pyo.SolverStatus.ok and other_condition)  # GAP limit in SCIP

if infeasible or infeasible_or_unbounded or unbounded or unknown:
    log_infeasible_constraints(
        model,
        logger=logger,
        log_expression=True,
        log_variables=True,
    )

if feasible or optimal:
    print('Objective: {}'.format(pyo.value(model.cost)))
    # Print the optimal solution
    for node_i in nodes:
        for node_j in nodes:
            for vehicle in vehicles:
                if model.x[node_i, node_j, vehicle].value == 1:
                    print(f"Vehicle {vehicle} travels from {node_i} to {node_j}")
    for node_j in nodes[1:]:
        print(f"Arrival time at {node_j}: {model.u[node_j].value}")
