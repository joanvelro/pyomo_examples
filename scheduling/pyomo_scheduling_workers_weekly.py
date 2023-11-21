import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# Define days (1 week)
days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
# Enter shifts of each day
shifts = ['morning', 'evening', 'night']  # 3 shifts of 8 hours
days_shifts = {day: shifts for day in days}  # dict with day as key and list of its shifts as value
# Enter workers ids (name, number, ...)
workers = ['Worker-' + str(i) for i in range(1, 11)]  # 10 workers available, more than needed

# Initialize model
model = pyo.ConcreteModel()

model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)

# binary variables representing if a worker is scheduled somewhere
model.works = pyo.Var(((worker, day, shift)
                       for worker in workers
                       for day in days
                       for shift in days_shifts[day]), within=pyo.Binary, initialize=0)

# binary variables representing if a worker is necessary
model.needed = pyo.Var(workers, within=pyo.Binary, initialize=0)

# binary variables representing if a worker worked on sunday but not on saturday (avoid if possible)
model.no_pref = pyo.Var(workers, within=pyo.Binary, initialize=0)


# Define an objective function with model as input, to pass later
def obj_rule(model):
    c = len(workers)
    return sum(model.no_pref[worker] for worker in workers) + sum(c * model.needed[worker] for worker in workers)


# we multiply the second term by a constant to make sure that it is the primary objective
# since sum(m.no_prefer) is at most len(workers), len(workers) + 1 is a valid constant.


# add objective function to the model. rule (pass function) or expr (pass expression directly)
model.obj = pyo.Objective(rule=obj_rule, sense=pyo.minimize)

model.constraints = pyo.ConstraintList()  # Create a set of constraints

# Constraint: all shifts are assigned
for day in days:
    for shift in days_shifts[day]:
        if day in days[:-1] and shift in ['morning', 'evening']:
            # weekdays' and Saturdays' day shifts have exactly two workers
            # to add a constraint to model.constraints set
            model.constraints.add(2 == sum(model.works[worker, day, shift] for worker in workers))
        else:
            # Sundays' and nights' shifts have exactly one worker
            model.constraints.add(1 == sum(model.works[worker, day, shift] for worker in workers))

# Constraint: no more than 40 hours worked
for worker in workers:
    model.constraints.add(
        40 >= sum(8 * model.works[worker, day, shift] for day in days for shift in days_shifts[day])
    )

# Constraint: rest between two shifts is of 12 hours (i.e., at least two shifts)
for worker in workers:
    for j in range(len(days)):
        # if working in morning, cannot work again that day
        model.constraints.add(
            1 >= sum(model.works[worker, days[j], shift] for shift in days_shifts[days[j]])
        )
        # if working in evening, until next evening (note that after sunday comes next monday)
        model.constraints.add(
            1 >= sum(model.works[worker, days[j], shift] for shift in ['evening', 'night']) +
            model.works[worker, days[(j + 1) % 7], 'morning']
        )
        # if working in night, until next night
        model.constraints.add(
            1 >= model.works[worker, days[j], 'night'] +
            sum(model.works[worker, days[(j + 1) % 7], shift] for shift in ['morning', 'evening'])
        )

# Constraint (def of model.needed)
# if any model.works[worker, ·, ·] non-zero, model.needed[worker] must be one; else is zero to reduce the obj function
for worker in workers:
    model.constraints.add(
        10000 * model.needed[worker] >= sum(
            model.works[worker, day, shift] for day in days for shift in days_shifts[day])
    )
    # 10000 is to remark, but 5 was enough since max of 40 hours yields max of 5 shifts, the maximum possible sum

# Constraint (def of model.no_pref)
for worker in workers:
    model.constraints.add(
        model.no_pref[worker] >= sum(model.works[worker, 'Sat', shift] for shift in days_shifts['Sat'])
        - sum(model.works[worker, 'Sun', shift] for shift in days_shifts['Sun'])
    )  # if not working on sunday but working saturday model.needed must be 1; else will be zero to reduce the obj
    # function

# print(model.pprint())
# Choose Solver

opt = SolverFactory('scip')  # cbc

# Solve Model
results = opt.solve(model, tee=True)

def get_workers_needed(needed):
    """Extract to a list the needed workers for the optimal solution."""
    workers_needed = []
    for worker in workers:
        if needed[worker].value == 1:
            workers_needed.append(worker)
    return workers_needed


def get_work_table(works):
    """Build a timetable of the week as a dictionary from the model's optimal solution."""
    week_table = {day: {shift: [] for shift in days_shifts[day]} for day in days}
    for worker in workers:
        for day in days:
            for shift in days_shifts[day]:
                if works[worker, day, shift].value == 1:
                    week_table[day][shift].append(worker)
    return week_table


def get_no_preference(no_pref):
    """Extract to a list the workers not satisfied with their weekend preference."""
    return [worker for worker in workers if no_pref[worker].value == 1]


# Check the solution
if (results.solver.status == SolverStatus.ok) and (
        results.solver.termination_condition == TerminationCondition.optimal):
    print(':::Feasible solution:::')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')

    print(results.Solver())
    workers_needed = get_workers_needed(model.needed)  # dict with the optimal timetable
    print(workers_needed)
    week_table = get_work_table(model.works)  # list with the required workers
    print(week_table)
    # list with the non-satisfied workers (work on Sat but not on Sun)
    workers_no_pref = get_no_preference(model.no_pref)
    print(workers_no_pref)

    # print('--- Dual Variables ---')
    # print(model.dual.pprint())

    # print(model.constraints.display())

    # Do something when the solution in optimal and feasible
elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('(!) Infeasible solution')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)


