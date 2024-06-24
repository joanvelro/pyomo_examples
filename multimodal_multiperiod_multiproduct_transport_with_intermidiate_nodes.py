from pyomo.environ import *
from pyomo.opt import SolverFactory

# Define the model
model = ConcreteModel()

# Sets
model.products = Set(initialize=['P1', 'P2'])  # Products
model.nodes = Set(
    initialize=['A', 'B', 'C', 'D'])  # Nodes (A is the source, D is the destination, B and C are intermediates)
model.periods = Set(initialize=[1, 2, 3])  # Time periods
model.modes = Set(initialize=['Truck', 'Train'])  # Transport modes

# Parameters
demand = {('D', 'P1', 3): 50,
          ('D', 'P2', 3): 60}  # Demand at node D for products P1 and P2 in period 3

supply = {('A', 'P1', 1): 50,
          ('A', 'P2', 1): 60}  # Supply at node A for products P1 and P2 in period 1
capacity = {('A', 'B', 'Truck'): 100,
            ('A', 'B', 'Train'): 150,
            ('B', 'C', 'Truck'): 100,
            ('B', 'C', 'Train'): 150,
            ('C', 'D', 'Truck'): 100,
            ('C', 'D', 'Train'): 150}  # Capacity for each route and mode
cost = {('A', 'B', 'Truck'): 10,
        ('A', 'B', 'Train'): 5,
        ('B', 'C', 'Truck'): 8,
        ('B', 'C', 'Train'): 4,
        ('C', 'D', 'Truck'): 7,
        ('C', 'D', 'Train'): 3}  # Cost per unit for each route and mode
time_window = {('B', 'P1', 1): (1, 2),
               ('B', 'P2', 1): (1, 2),
               ('C', 'P1', 2): (2, 3),
               ('C', 'P2', 2): (2, 3)}

# Variables
model.flow = Var(model.nodes, model.nodes, model.products, model.periods, model.modes, domain=NonNegativeReals)


# Objective: Minimize total transportation cost
def objective_rule(model):
    return sum(cost[n1, n2, m] * model.flow[n1, n2, p, t, m] for n1 in model.nodes for n2 in model.nodes
               for p in model.products for t in model.periods for m in model.modes if (n1, n2, m) in cost)


model.objective = Objective(rule=objective_rule, sense=minimize)


# Constraints

# Supply constraints
def supply_rule(model, n, p, t):
    if (n, p, t) in supply:
        return sum(model.flow[n, n2, p, t, m] for n2 in model.nodes for m in model.modes if (n, n2, m) in capacity) <= \
            supply[n, p, t]
    else:
        return Constraint.Skip


model.supply_constraint = Constraint(model.nodes, model.products, model.periods, rule=supply_rule)


# Demand constraints
def demand_rule(model, n, p, t):
    if (n, p, t) in demand:
        return sum(model.flow[n1, n, p, t, m] for n1 in model.nodes for m in model.modes if (n1, n, m) in capacity) >= \
            demand[n, p, t]
    else:
        return Constraint.Skip


model.demand_constraint = Constraint(model.nodes, model.products, model.periods, rule=demand_rule)


# Capacity constraints
def capacity_rule(model, n1, n2, m):
    if (n1, n2, m) in capacity:
        return sum(model.flow[n1, n2, p, t, m] for p in model.products for t in model.periods) <= capacity[n1, n2, m]
    else:
        return Constraint.Skip


model.capacity_constraint = Constraint(model.nodes, model.nodes, model.modes, rule=capacity_rule)


# Flow balance constraints
def flow_balance_rule(model, n, p, t):
    if n != 'A' and n != 'D':  # Intermediate nodes
        return sum(model.flow[n1, n, p, t, m] for n1 in model.nodes for m in model.modes if (n1, n, m) in capacity) == \
            sum(model.flow[n, n2, p, t, m] for n2 in model.nodes for m in model.modes if (n, n2, m) in capacity)
    else:
        return Constraint.Skip


model.flow_balance_constraint = Constraint(model.nodes, model.products, model.periods, rule=flow_balance_rule)


# Time window constraints
def time_window_rule(model, n, p, t):
    if (n, p, t) in time_window:
        min_time, max_time = time_window[n, p, t]
        return sum(model.flow[n1, n, p, t2, m] for n1 in model.nodes for t2 in model.periods for m in model.modes
                   if (n1, n, m) in capacity and t2 >= min_time and t2 <= max_time) >= 0
    else:
        return Constraint.Skip


model.time_window_constraint = Constraint(model.nodes, model.products, model.periods, rule=time_window_rule)

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model)

# Display results
print("Optimal Flow:")
for n1 in model.nodes:
    for n2 in model.nodes:
        for p in model.products:
            for t in model.periods:
                for m in model.modes:
                    if (n1, n2, m) in capacity and model.flow[n1, n2, p, t, m].value > 0:
                        print(f"From {n1} to {n2} product {p} period {t} mode {m}: {model.flow[n1, n2, p, t, m].value}")

print(f"Total Cost: {model.objective():.2f}")
