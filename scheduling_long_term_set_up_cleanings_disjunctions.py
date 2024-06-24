from pyomo.environ import *
from pyomo.gdp import Disjunction
from pyomo.core import TransformationFactory
# Define the model
model = ConcreteModel()

# Sets and Parameters
num_tasks = 5  # Number of tasks
tasks = range(1, num_tasks + 1)

# Task durations (example data in days)
task_durations = {1: 1, 2: 2, 3: 3, 4: 1, 5: 2}

# Set-up and maintenance tasks that depend on production time (example data)
depends_on_production = {
    3: (1, 2),  # Task 3 depends on tasks 1 and 2 being in production
    5: (4,)  # Task 5 depends on task 4 being in production
}

# Disjunction pairs (tasks that cannot overlap)
disjunction_pairs = [(1, 2), (3, 4)]

# Time horizon (example: 5 days)
horizon = 5

# Variables
model.start_time = Var(tasks, within=NonNegativeIntegers, bounds=(0, horizon))

model.var_max = Var(within=NonNegativeIntegers)

model.constraint_var_max = Constraint(expr=model.var_max >= sum(model.start_time[i] + task_durations[i] for i in tasks))

# Objective function (minimize makespan, for example)
model.makespan = Objective(
    expr=model.var_max,
    sense=minimize
)


# Constraints
def dependency_constraint_rule(model, task):
    if task in depends_on_production:
        return model.start_time[task] >= sum(
            model.start_time[dep] + task_durations[dep] for dep in depends_on_production[task])
    else:
        return Constraint.Skip


model.dependency_constraint = Constraint(tasks, rule=dependency_constraint_rule)


def disjunction_constraint_rule(model, task1, task2):
    left_side_conditions = model.start_time[task1] + task_durations[task1] <= model.start_time[task2]

    right_side_conditions = model.start_time[task2] + task_durations[task2] <= model.start_time[task1]

    return [left_side_conditions, right_side_conditions]


# model.disjunction_constraint = ConstraintList()
# for task1, task2 in disjunction_pairs:
#     model.disjunction_constraint.add(disjunction_constraint_rule(model, task1, task2))


model.disjunction_constraint = Disjunction(disjunction_pairs,
                                           rule=disjunction_constraint_rule)

TransformationFactory('gdp.hull').apply_to(model)

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model, tee=True)

# Display results
print("Optimal Schedule:")
for i in tasks:
    print(
        f"Task {i}: Start time = {model.start_time[i].value}, End time = {model.start_time[i].value + task_durations[i]}")

print(f"Optimal Makespan: {model.makespan():.2f} days")
