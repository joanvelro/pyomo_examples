from pyomo.environ import *
from pyomo.core import TransformationFactory
from pyomo.gdp import Disjunction

# Define the model
model = ConcreteModel()

# Sets and Parameters
num_products = 2  # Number of products
products = range(1, num_products + 1)

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

# Time horizon (example: 30 days)
horizon = 30

# Variables
model.start_time = Var(products, tasks, within=NonNegativeIntegers, bounds=(0, horizon))

model.var_max = Var(within=NonNegativeIntegers)

model.constraint_var_max = Constraint(
    expr=model.var_max >= sum(model.start_time[p, t] + task_durations[t] for p in products for t in tasks))

# Objective function (minimize makespan, for example)
model.makespan = Objective(
    expr=model.var_max,
    sense=minimize
)


# Constraints
def dependency_constraint_rule(model, p, t):
    if t in depends_on_production:
        return model.start_time[p, t] >= sum(
            model.start_time[p, dep] + task_durations[dep] for dep in depends_on_production[t])
    else:
        return Constraint.Skip


model.dependency_constraint = Constraint(products, tasks, rule=dependency_constraint_rule)


def disjunction_constraint_rule(model, p, task1, task2):
    left_side = model.start_time[p, task1] + task_durations[task1] <= model.start_time[p, task2]
    right_side = model.start_time[p, task2] + task_durations[task2] <= model.start_time[p, task1]
    return [left_side, right_side]


model.disjunction_constraint = Disjunction(products,
                                           disjunction_pairs,
                                           rule=disjunction_constraint_rule)

TransformationFactory('gdp.hull').apply_to(model)

# Solve the model
solver = SolverFactory('glpk')
solver.solve(model, tee=True)

# Display results
print("Optimal Schedule:")
for p in products:
    print(f"Product {p}:")
    for t in tasks:
        print(
            f"  Task {t}: Start time = {model.start_time[p, t].value}, End time = {model.start_time[p, t].value + task_durations[t]}")

print(f"Optimal Makespan: {model.makespan():.2f} days")
