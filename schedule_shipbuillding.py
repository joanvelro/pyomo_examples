from pyomo.environ import *

from utils import has_solution

# Define the model
model = ConcreteModel()

tasks = ['Hull',
         'Engine',
         'Deck',
         'Interior',
         'Electronics']
workstations = ['Station1',
                'Station2',
                'Station3',
                'Station4',
                'Station5']
# Sets
model.tasks = Set(initialize=tasks)
model.workstations = Set(initialize=workstations)

# Parameters
# Time required to complete each task (in hours)
task_times = {
    ('Hull', 'Station1'): 50,
    ('Hull', 'Station2'): 55,
    ('Hull', 'Station3'): 60,
    ('Hull', 'Station4'): 15,
    ('Hull', 'Station5'): 15,
    ('Engine', 'Station1'): 40,
    ('Engine', 'Station2'): 35,
    ('Engine', 'Station3'): 45,
    ('Engine', 'Station4'): 50,
    ('Engine', 'Station5'): 50,
    ('Deck', 'Station1'): 30,
    ('Deck', 'Station2'): 25,
    ('Deck', 'Station3'): 35,
    ('Deck', 'Station4'): 40,
    ('Deck', 'Station5'): 40,
    ('Interior', 'Station1'): 20,
    ('Interior', 'Station2'): 15,
    ('Interior', 'Station3'): 5,
    ('Interior', 'Station4'): 30,
    ('Interior', 'Station5'): 30,
    ('Electronics', 'Station1'): 10,
    ('Electronics', 'Station2'): 55,
    ('Electronics', 'Station3'): 5,
    ('Electronics', 'Station4'): 20,
    ('Electronics', 'Station5'): 20,
}

# model.task_times = Param(model.tasks,
#                          model.workstations,
#                          initialize=task_times)

# Binary variable to indicate if a task is assigned to a workstation
model.x = Var(model.tasks,
              model.workstations,
              domain=Binary,
              initialize=0)

model.slack = Var(
    model.workstations,
    domain=Binary,
    initialize=0)

model.max_x = Var(model.workstations,
                  domain=NonNegativeIntegers,
                  initialize=0)


# A workstation can handle only one task at a time
def rule_constraint_max_linearization(model, w):
    working_time_workstation = sum(model.x[t, w] * task_times[(t, w)]
                                   for t in model.tasks)
    return model.max_x[w] >= working_time_workstation


model.constraint_max_linearization = Constraint(model.workstations, rule=rule_constraint_max_linearization)


# Constraints

# Each task must be assigned to exactly one workstation
def task_assignment_rule(model, t):
    return sum(model.x[t, w] for w in model.workstations) == 1


model.task_assignment = Constraint(model.tasks, rule=task_assignment_rule)


# A workstation can handle only one task at a time
def workstation_rule(model, w):
    all_tasks_assigned = sum(model.x[t, w] for t in model.tasks)
    return all_tasks_assigned <= 1 + model.slack[w]


model.workstation_constraint = Constraint(model.workstations, rule=workstation_rule)


# def _feasibility_constraint(model):
#     return model.x['Deck', 'Station3']==1
#
# model.feasibility_constraint = Constraint(rule=_feasibility_constraint)


# Objective: Minimize the maximum completion time (makespan)
def makespan_rule(model):
    slack_objective = sum(model.slack[w]
                          for w in model.workstations)
    decision_objective = sum(model.max_x[w]
                             for w in model.workstations)
    return decision_objective + slack_objective


model.objective_minimize_makespan = Objective(rule=makespan_rule, sense=minimize)
model.objective_minimize_makespan.activate()

model.objective_dummy = Objective(rule=0, sense=minimize)
model.objective_dummy.deactivate()

# Solve the model
model.constraint_max_linearization.activate()
model.task_assignment.activate()
model.workstation_constraint.activate()
solver = SolverFactory('cbc')
results = solver.solve(model, tee=True)

if has_solution(results, model):

    for ((t, w), var) in model.x.items():
        if value(var) > 0:
            print('task: {} --> workstation: {}'.format(t, w))

    for ((w), var) in model.slack.items():
        if value(var) > 0:
            print('slack workstation: {}'.format(w))

    print(f"Minimum makespan: {model.objective_minimize_makespan()} hours")
