from pyomo.environ import (
    ConcreteModel,
    Var,
    Objective,
    Constraint,
    NonNegativeReals,
    Binary,
    minimize,
    RangeSet)

from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
# Define the model
model = ConcreteModel()

# Parameters
num_machines = 3  # Number of machines
T = 10  # Total time horizon (e.g., days)
C_p = 5  # Cost of production per unit time
C_m = 10  # Cost of maintenance per unit time
M = [6, 8, 7]  # Maximum allowable cumulative production time for each machine before maintenance
tau = 5  # Frequency of periodic maintenance

# Sets
model.machines = RangeSet(num_machines)
model.time = RangeSet(1, T)

# Decision Variables
model.x = Var(model.machines, model.time, domain=Binary)  # 1 if machine i is producing at time t, 0 otherwise
model.y = Var(model.machines, model.time, domain=Binary)  # 1 if machine i is under maintenance at time t, 0 otherwise
model.P = Var(model.machines, model.time,
              domain=NonNegativeReals)  # Cumulative production time for machine i up to time t


# Objective Function: Minimize total cost
def total_cost(model):
    return sum(C_p * model.x[machine, period]
    for machine in model.machines
    for period in model.time) + sum(C_m*model.y[machine, period]
                                    for machine in model.machines
                                    for period in model.time)


model.obj = Objective(rule=total_cost, sense=minimize)


# Constraints

# Production and Maintenance Constraint
def prod_maint_constraint(model, i, t):
    return model.x[i, t] + model.y[i, t] <= 1


model.prod_maint_constr = Constraint(model.machines, model.time, rule=prod_maint_constraint)


# Cumulative Production Time Constraint
def cumulative_production_time_constraint(model, i, t):
    if t == 1:
        return model.P[i, t] == model.x[i, t]
    else:
        return model.P[i, t] == model.P[i, t - 1] + model.x[i, t] - M[i - 1] * model.y[i, t]


model.cumulative_prod_time_constr = Constraint(model.machines, model.time, rule=cumulative_production_time_constraint)


# Maintenance Trigger Constraint
def maintenance_trigger_constraint(model, i, t):
    return model.P[i, t] <= M[i - 1]


model.maintenance_trigger_constr = Constraint(model.machines, model.time,
                                              rule=maintenance_trigger_constraint)


# Periodic Maintenance Constraint
def periodic_maintenance_constraint(model, machine, period):
    if period % tau == 0:
        return model.y[machine, period] == 1
    else:
        return Constraint.Skip


model.periodic_maintenance_constr = Constraint(model.machines, model.time,
                                               rule=periodic_maintenance_constraint)

model.obj.activate()
model.prod_maint_constr.activate()
model.cumulative_prod_time_constr.activate()
model.maintenance_trigger_constr.activate()
model.periodic_maintenance_constr.deactivate()

# Solve the model
solver = SolverFactory('glpk')
results = solver.solve(model)


# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print('--- Optimal solution ---')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())
    model.pprint()

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)


