from pandas import DataFrame, merge
import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

enable_recirculation = True
enable_capacity = True

# Create a Concrete Model
model = pyo.ConcreteModel()

# Sets
set_products = ['Juice', 'Carbonated Drink', 'Snack']
set_periods = ['Period1', 'Period2', 'Period3']

# Parameters
param_demand = {'Juice': 3500,
                'Carbonated Drink': 1500,
                'Snack': 2000}

# units per hour produced
param_productivity = {'Juice': 350,  # units/period
                      'Carbonated Drink': 550,
                      'Snack': 400}

param_recirculation_time = {'Juice': 2,
                            'Carbonated Drink': 3,
                            'Snack': 1}

# Hours that can be produced with in each period
param_capacity = {'Period1': 5,
                  'Period2': 8,
                  'Period3': 10}

# Variables
# hour to begin and finish
model.var_start_time = pyo.Var(set_products, set_periods, within=pyo.NonNegativeIntegers)
model.var_end_time = pyo.Var(set_products, set_periods, within=pyo.NonNegativeIntegers)
model.var_recirculation_start = pyo.Var(set_products, set_periods, within=pyo.NonNegativeIntegers)
model.var_recirculation_end = pyo.Var(set_products, set_periods, within=pyo.NonNegativeIntegers)

# Objective function
model.obj = pyo.Objective(expr=sum(model.var_end_time[p, s] for p in set_products for s in set_periods),
                          sense=pyo.minimize)

# Constraints
# model.demand_constraint = pyo.ConstraintList()
model.recirculation_constraint = pyo.ConstraintList()


def production_rule(model, p, s):
    return (model.var_end_time[p, s] - model.var_start_time[p, s]) * param_productivity[p]


model.production = pyo.Expression(set_products, set_periods, rule=production_rule)


def demand_constraint_rule(model, p):
    return sum(model.production[p, s] for s in set_periods) >= param_demand[p]


model.demand_constraint = pyo.Constraint(set_products, rule=demand_constraint_rule)

# # Demand constraint
# for p in set_products:
#     model.demand_constraint.add(sum(model.var_end_time[p, s] for s in set_periods) >= param_demand[p])

if enable_recirculation:
    # Recirculation constraint
    for p in set_products:
        for s in set_periods:
            model.recirculation_constraint.add(
                model.var_recirculation_end[p, s] - model.var_recirculation_start[p, s] >=
                param_recirculation_time[p])
            model.recirculation_constraint.add(model.var_recirculation_end[p, s] >= model.var_start_time[p, s])


def demand_constraint_rule(model, s):
    return sum(model.var_end_time[p, s] - model.var_start_time[p, s] for p in set_products) <= param_capacity[s]


if enable_capacity:
    model.capacity_constraint = pyo.Constraint(set_periods, rule=demand_constraint_rule)

# Solve the optimization problem
solver = SolverFactory('cbc')

results = solver.solve(model, tee=True)
status_ok = results.solver.status == SolverStatus.ok
feasible = results.solver.termination_condition == TerminationCondition.feasible
infeasible = results.solver.termination_condition == TerminationCondition.infeasible
optimal = results.solver.termination_condition == TerminationCondition.optimal
if status_ok and (feasible or optimal):
    print('\n')
    if feasible:
        print('FEASIBLE Solution founded!')
    if optimal:
        print('OPTIMAL Solution founded!')
    print('\n')
    start_time = []
    end_time = []
    recirculation_start = []
    recirculation_end = []
    # Print the results

    for ((p, s), var) in model.production.items():
        value_ = pyo.value(var)
        print('production({},{}):{}'.format(p, s, value_))

    print('\n')
    for p in set_products:
        for s in set_periods:
            start_time_value = model.var_start_time[p, s].value
            end_time_value = model.var_end_time[p, s].value

            start_time.append(tuple((p, s, start_time_value)))
            end_time.append(tuple((p, s, end_time_value)))

            if enable_recirculation:
                recirculation_start_value = model.var_recirculation_start[p, s].value
                recirculation_end_value = model.var_recirculation_end[p, s].value

                recirculation_start.append(tuple((p, s, recirculation_start_value)))
                recirculation_end.append(tuple((p, s, recirculation_end_value)))

    df_start_time = DataFrame(start_time, columns=['PRODUCT', 'STAGE', 'START'])
    df_end_time = DataFrame(end_time, columns=['PRODUCT', 'STAGE', 'END'])
    df_starts = merge(df_start_time, df_end_time, on=['PRODUCT', 'STAGE'], how='inner')

    print('Start-ups and Shut-down')
    for _, row in df_starts.iterrows():
        print('Product:{} Stage:{} Start:{} End:{}'.format(row['PRODUCT'], row['STAGE'], row['START'], row['END']))

    if enable_recirculation:
        print('\n')
        print('Recirculation')
        df_recirculation_start = DataFrame(recirculation_start, columns=['PRODUCT', 'STAGE', 'REC_START'])
        df_recirculation_end = DataFrame(recirculation_end, columns=['PRODUCT', 'STAGE', 'REC_END'])
        df_recirculation = merge(df_recirculation_start, df_recirculation_end, on=['PRODUCT', 'STAGE'], how='inner')

        for _, row in df_recirculation.iterrows():
            print('Product:{} Stage:{} Start:{} End:{}'.format(row['PRODUCT'], row['STAGE'], row['REC_START'],
                                                               row['REC_END']))
        print('\n')

elif infeasible:
    print('Infeasible solution')
else:
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)

print('hi')
