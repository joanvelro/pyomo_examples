from pyomo.environ import *


# Constants
N = ['depot', 'A', 'B', 'C', 'D', 'E']  # Nodes including depot
A = [('depot', 'A'), ('depot', 'B'), ('A', 'C'), ('B', 'D'), ('C', 'E'), ('D', 'E')]  # Arcs
D = ['Truck1', 'Truck2']  # Set of trucks

# Parameters
early = {'depot': 0, 'A': 1, 'B': 1, 'C': 3, 'D': 2, 'E': 4}  # Early time window
late = {'depot': 999, 'A': 4, 'B': 5, 'C': 6, 'D': 7, 'E': 8}  # Late time window
service = {'depot': 0, 'A': 1, 'B': 1, 'C': 1, 'D': 1, 'E': 1}  # Service time at each node
c = {('depot', 'A'): 5, ('depot', 'B'): 10, ('A', 'C'): 7, ('B', 'D'): 12, ('C', 'E'): 6, ('D', 'E'): 9}  # Cost matrix

# Initialize concrete model
model = ConcreteModel()

# Decision variables
model.x = Var(A, D, within=Binary)
model.t = Var(N, D, within=NonNegativeReals)
model.y = Var(N, D, within=Binary)

# Objective function
def _objective_expression(model):
    return sum(model.x[i, j, d]
                                       for j in N
                                       for i in N
                                       if (i, j) in A
                                       for d in D)
model.obj = Objective(rule=_objective_expression, sense=minimize)

# Constraints
# Flow conservation constraints
model.flow_conservation = ConstraintList()
for d in D:
    for i in N:
        if i == 'depot':
            model.flow_conservation.add(expr=sum(model.x['depot', j, d]
                                                 for j in N
                                                 if (i, j) in A) == 1)
        elif i != 'depot':
            model.flow_conservation.add(expr=sum(model.x[i, j, d]
                                                 for j in N
                                                 if (i, j) in A) - sum(model.x[j, i, d]
                                                                                  for j in N
                                                                                  if (j, i) in A) == model.y[i, d] - 1)

# Time window constraints
model.time_window = ConstraintList()
M = 1000  # Large number
for (i, j) in A:
    for d in D:
        model.time_window.add(expr=model.t[j, d] >= model.t[i, d] + service[i] + c[i, j] - M * (1 - model.x[i, j, d]))
for i in N:
    for d in D:
        model.time_window.add(expr=early[i] <= model.t[i, d] <= late[i])

# Visit constraints
model.visit_constraints = ConstraintList()
for (i, j) in A:
    for d in D:
        model.visit_constraints.add(expr=model.t[i, d] >= model.t[j, d] + service[j] - M * (1 - model.y[i, d]))
for i in N:
    for d in D:
        model.visit_constraints.add(expr=model.t[i, d] >= early[i] * model.y[i, d])
model.visit_constraints.add(expr=sum(model.y[i, d] for i in N) == 1)

# Solve the model
solver = SolverFactory('glpk')
results = solver.solve(model)

# Print the results
print(results)

# Print optimal solution
print("\nOptimal Solution:")
for d in D:
    print(f"\nTruck {d}:")
    for i in N:
        for j in N:
            if model.x[i, j, d].value == 1:
                print(f"Travel from {i} to {j}")
    print(f"Total Cost: {model.obj():.2f}")