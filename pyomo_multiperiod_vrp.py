"""
This example formulates a multi-period multi-product VRP with time windows using Pyomo. It considers a set of nodes (depot and customers), a set of products, a set of vehicles, and a set of periods. The objective is to minimize the total travel time. The model includes decision variables to represent the transportation of products between nodes by vehicles in each period. It also includes constraints for flow conservation, capacity, time windows, and other problem-specific constraints. The model is solved using the GLPK solver, and the solution is printed, showing the optimal routes for each vehicle in each period.

"""
from pyomo.environ import *

# Sets
nodes = ['Depot', 'Customer1', 'Customer2', 'Customer3']  # Set of nodes (depot and customers)
nodes_ = ['Customer1', 'Customer2', 'Customer3']
products = ['Product1', 'Product2']  # Set of products
vehicles = ['Vehicle1', 'Vehicle2']  # Set of vehicles
periods = ['Period1', 'Period2']  # Set of periods

# Parameters
demand = {'Customer1': {'Product1': 10, 'Product2': 5},
          'Customer2': {'Product1': 8, 'Product2': 6},
          'Customer3': {'Product1': 12, 'Product2': 4}}  # Demand at each customer node for each product
time_windows = {'Customer1': (8, 12), 'Customer2': (9, 13),
                'Customer3': (10, 14)}  # Time windows for each customer node
vehicle_capacity = {'Vehicle1': 30, 'Vehicle2': 25}  # Capacity of each vehicle
service_time = {'Customer1': 2, 'Customer2': 3, 'Customer3': 2}  # Service time at each customer node
travel_time = {'Depot': {'Customer1': 10, 'Customer2': 15, 'Customer3': 20},
               'Customer1': {'Depot': 10, 'Customer2': 5, 'Customer3': 15},
               'Customer2': {'Depot': 15, 'Customer1': 5, 'Customer3': 10},
               'Customer3': {'Depot': 20, 'Customer1': 15, 'Customer2': 10}}  # Travel time between nodes

# Model
model = ConcreteModel()

# Decision Variables
model.x = Var(nodes, nodes, products, vehicles, periods,
              within=Binary)  # Binary variable indicating if a vehicle transports a product from one node to another in a period
model.u = Var(nodes, periods,
              within=NonNegativeReals)  # Variable representing the time at which a node is visited in a period

# Objective Function
model.cost = Objective(expr=sum(travel_time[i][j] * model.x[i, j, p, v, t]
                                for i in nodes
                                for j in nodes
                                if i != j
                                for p in products
                                for v in vehicles
                                for t in periods
                                ),
                       sense=minimize)

# Constraints
model.flow_constraint = ConstraintList()
for j in nodes[1:]:
    for p in products:
        for t in periods:
            model.flow_constraint.add(
                sum(model.x[i, j, p, v, t] for i in nodes for v in
                    vehicles) == 1)  # Each customer node is visited exactly once for each product in each period

model.depot_constraint = ConstraintList()
for v in vehicles:
    for p in products:
        for t in periods:
            model.depot_constraint.add(sum(model.x['Depot', j, p, v, t] for j in nodes[
                                                                                 1:]) == 1)  # Each vehicle departs from the depot for each product in each period

model.capacity_constraint = ConstraintList()
for v in vehicles:
    for p in products:
        for t in periods:
            for j in nodes[1:]:
                model.capacity_constraint.add(
                    sum(demand[i][p] * model.x[i, j, p, v, t] for i in nodes_) <= vehicle_capacity[v])  # Capacity constraint for each vehicle and product in each period

M = 1e9
model.time_window_constraint = ConstraintList()
for v_ in vehicles:
    for j in nodes_:
        for p in products:
            for t in periods:
                model.time_window_constraint.add(model.u[j, t] >= time_windows[j][0])  # Lower time window constraint
                model.time_window_constraint.add(model.u[j, t] <= time_windows[j][1])  # Upper time window constraint
                for i in nodes:
                    model.time_window_constraint.add(
                        model.u[j, t] >= model.u[i, t] + travel_time[i][j] - M * (1 - model.x[i, j, p, v_, t]))  # Time window constraint

# Solve the model
solver = SolverFactory('glpk')
results = solver.solve(model)

# Print the solution
print("Objective Value:", model.cost())
for i in nodes:
    for j in nodes:
        for p in products:
            for v in vehicles:
                for t in periods:
                    if model.x[i, j, p, v, t].value == 1:
                        print(f"Vehicle {v} transports {p} from {i} to {j} in period {t}")
