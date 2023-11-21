from pyomo.environ import *

# Sample data
num_tanks = 3
time_horizon = 5
fixed_cost = [100, 150, 200]
tank_capacity = [500, 700, 600]
min_inflow_rate = [0, 0, 0]
max_inflow_rate = [200, 300, 250]
min_outflow_rate = [0, 0, 0]
max_outflow_rate = [150, 250, 200]
initial_state = [1, 0, 1]

# Create Pyomo Concrete Model
model = ConcreteModel()


# Sample data for inflow_rate and outflow_rate
inflow_data = {
    (0, 0): 100, (0, 1): 150, (0, 2): 200,
    (1, 0): 120, (1, 1): 180, (1, 2): 250,
    (2, 0): 140, (2, 1): 200, (2, 2): 230,
    (3, 0): 180, (3, 1): 220, (3, 2): 200,
    (4, 0): 160, (4, 1): 190, (4, 2): 260
}

outflow_data = {
    (0, 0): 80, (0, 1): 120, (0, 2): 180,
    (1, 0): 100, (1, 1): 150, (1, 2): 210,
    (2, 0): 110, (2, 1): 160, (2, 2): 190,
    (3, 0): 150, (3, 1): 180, (3, 2): 170,
    (4, 0): 130, (4, 1): 160, (4, 2): 220
}

# Add inflow_rate and outflow_rate to the model
model.inflow_rate = Param(range(time_horizon), range(num_tanks), initialize=inflow_data)
model.outflow_rate = Param(range(time_horizon), range(num_tanks), initialize=outflow_data)

# Define decision variables
model.x = Var(range(num_tanks),
              range(time_horizon),
              within=Binary)


# Objective function
model.cost = Objective(expr=sum(model.x[i, t] * fixed_cost[i] for i in range(num_tanks) for t in range(time_horizon)),
                       sense=minimize)

# Constraints
model.capacity_constraints = ConstraintList()
model.flow_rate_constraints = ConstraintList()
model.enablement_constraints = ConstraintList()

for t in range(time_horizon):
    for i in range(num_tanks):
        model.capacity_constraints.add(model.x[i, t] * min_inflow_rate[i] <= model.x[i, t] * model.inflow_rate[t, i])
        model.capacity_constraints.add(model.x[i, t] * model.inflow_rate[t, i] <= model.x[i, t] * max_inflow_rate[i])
        model.capacity_constraints.add(model.x[i, t] * model.outflow_rate[t, i] <= model.x[i, t] * max_outflow_rate[i])
        model.capacity_constraints.add(model.x[i, t] * min_outflow_rate[i] <= model.x[i, t] * model.outflow_rate[t, i])
        model.capacity_constraints.add(
            sum(model.x[i, k] * (model.inflow_rate[t, i] - model.outflow_rate[t, i]) for k in range(t + 1)) <=
            model.x[i, t] * (tank_capacity[i] - initial_state[i]))


for t in range(time_horizon):
    for i in range(num_tanks):
        if t-1 >= 0:
            model.enablement_constraints.add(model.x[i, t] - model.x[i, t - 1] <= 1)
            model.enablement_constraints.add(model.x[i, t] - model.x[i, t - 1] >= -1)

# Solver
solver = SolverFactory('scip')


# Solve the optimization problem
results = solver.solve(model, tee=True)

# Check the optimization results
if results.solver.termination_condition == TerminationCondition.optimal:
    print("Optimal Solution Found!")
    model.pprint()
    for t in range(time_horizon):
        for i in range(num_tanks):
            if value(model.x[i, t]) == 1:
                print(f"Tank {i + 1} is enabled at time step {t + 1}.")
else:
    print("No optimal solution found.")
