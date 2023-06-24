"""
Script de un modelo muy simplificado del MVP de GEA como primer paso para la implementación del modelo total
"""
import pyomo.environ
import numpy
import pandas
import matplotlib
import matplotlib.pyplot

"""
Creacion de una lista de maquinas disponibles y de las matrices minima y maxima que definen los caminos posibles para 
las recetas.
"""

available_machines = ['tank_a', 'tank_b', 'pasteurizer', 'crystallization_tank_1', 'crystallization_tank_2',
                      'out_1a', 'out_1b', 'out_2a', 'out_2b']
products = ['alice', 'bob']
min_matrix_recipe = {(m, n, k): 0 for m in available_machines for n in available_machines for k in products}
min_matrix_recipe['tank_a', 'pasteurizer', 'alice'] = 1
min_matrix_recipe['pasteurizer', 'out_1a', 'alice'] = 1
min_matrix_recipe['tank_b', 'pasteurizer', 'bob'] = 1
min_matrix_recipe['pasteurizer', 'out_1b', 'bob'] = 1
max_matrix_recipe = min_matrix_recipe.copy()
max_matrix_recipe['pasteurizer', 'crystallization_tank_1', 'alice'] = 1
max_matrix_recipe['pasteurizer', 'crystallization_tank_2', 'alice'] = 1
max_matrix_recipe['crystallization_tank_1', 'out_2a', 'alice'] = 1
max_matrix_recipe['crystallization_tank_2', 'out_2a', 'alice'] = 1
max_matrix_recipe['pasteurizer', 'crystallization_tank_1', 'bob'] = 1
max_matrix_recipe['pasteurizer', 'crystallization_tank_2', 'bob'] = 1
max_matrix_recipe['crystallization_tank_1', 'out_2b', 'bob'] = 1
max_matrix_recipe['crystallization_tank_2', 'out_2b', 'bob'] = 1
machine_pair_durations = {(m, n): 0 for m in available_machines for n in available_machines}
machine_pair_durations['tank_a', 'pasteurizer'] = 3600
machine_pair_durations['tank_b', 'pasteurizer'] = 3600
machine_pair_durations['pasteurizer', 'crystallization_tank_1'] = 3600
machine_pair_durations['pasteurizer', 'crystallization_tank_2'] = 3600
machine_pair_durations['pasteurizer', 'out_1a'] = 3600
machine_pair_durations['pasteurizer', 'out_1b'] = 3600
machine_pair_durations['crystallization_tank_1', 'out_2a'] = 3600
machine_pair_durations['crystallization_tank_1', 'out_2b'] = 3600
machine_pair_durations['crystallization_tank_2', 'out_2a'] = 3600
machine_pair_durations['crystallization_tank_2', 'out_2b'] = 3600
machine_process_durations = {m: 0 for m in available_machines}
machine_process_durations['crystallization_tank_1'] = 28800
machine_process_durations['crystallization_tank_2'] = 28800
is_batch_process = {(m, n): 0 for m in available_machines for n in available_machines}
is_batch_process['pasteurizer', 'crystallization_tank_1'] = 1
is_batch_process['pasteurizer', 'crystallization_tank_2'] = 1

"""
Creación del modelo de pyomo
"""
model = pyomo.environ.ConcreteModel('dummy model v2')

model.machines = pyomo.environ.Set(initialize=available_machines)
model.products = pyomo.environ.Set(initialize=products)

model.batch_sizes_kg = pyomo.environ.Param(model.products, initialize={'alice': 15000, 'bob': 15000},
                                           within=pyomo.environ.Reals)
model.order_kg = pyomo.environ.Param(model.products, initialize={'alice': 3e4, 'bob': 1.5e4},
                                     within=pyomo.environ.Reals)

model.number_batches = pyomo.environ.Param(
    model.products, initialize={i: int(numpy.ceil(model.order_kg[i] / model.batch_sizes_kg[i])) for i in products},
    within=pyomo.environ.Integers)

model.total_batch_index = pyomo.environ.Set(initialize=list(range(sum(model.number_batches[i] for i in products))))

model.batch_product = pyomo.environ.Var(model.total_batch_index, model.products, within=pyomo.environ.Binary)


def constraint_only_one_product_per_batch(model, i):
    return sum(model.batch_product[i, k] for k in model.products) == 1


model.constr1 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_only_one_product_per_batch)


def constraint_total_number_of_batches_per_product(model, k):
    return sum(model.batch_product[i, k] for i in model.total_batch_index) == model.number_batches[k]


model.constr2 = pyomo.environ.Constraint(model.products, rule=constraint_total_number_of_batches_per_product)

model.min_matrix_recipe = pyomo.environ.Param(model.machines, model.machines, model.products,
                                              initialize=min_matrix_recipe, within=pyomo.environ.Binary)
model.max_matrix_recipe = pyomo.environ.Param(model.machines, model.machines, model.products,
                                              initialize=max_matrix_recipe, within=pyomo.environ.Binary)
model.matrix_recipe = pyomo.environ.Var(model.total_batch_index, model.machines, model.machines,
                                        within=pyomo.environ.Binary)


def constraint_minimum_recipe(model, i, m, n):
    return sum(model.min_matrix_recipe[m, n, k] * model.batch_product[i, k] for k in model.products) \
           <= model.matrix_recipe[i, m, n]


model.constr3 = pyomo.environ.Constraint(model.total_batch_index, model.machines, model.machines,
                                         rule=constraint_minimum_recipe)


def constraint_maximum_recipe(model, i, m, n):
    return sum(model.max_matrix_recipe[m, n, k] * model.batch_product[i, k] for k in model.products) \
           >= model.matrix_recipe[i, m, n]


model.constr4 = pyomo.environ.Constraint(model.total_batch_index, model.machines, model.machines,
                                         rule=constraint_maximum_recipe)


def constraint_at_least_one_crystallizer(model, i):
    return model.matrix_recipe[i, 'pasteurizer', 'crystallization_tank_1'] + \
           model.matrix_recipe[i, 'pasteurizer', 'crystallization_tank_2'] >= 1


model.constr5 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_at_least_one_crystallizer)


def constraint_output_from_crystallizer_1_1(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2a'] >= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_1'] + \
           model.batch_product[i, 'alice'] - 1


model.constr6 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_1_1)


def constraint_output_from_crystallizer_1_2(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2a'] <= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_1']


model.constr7 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_1_2)


def constraint_output_from_crystallizer_1_3(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2a'] <= model.batch_product[i, 'alice']


model.constr8 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_1_3)


def constraint_output_from_crystallizer_2_1(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2a'] >= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_2'] + \
           model.batch_product[i, 'alice'] - 1


model.constr9 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_2_1)


def constraint_output_from_crystallizer_2_2(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2a'] <= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_2']


model.constr10 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_2_2)


def constraint_output_from_crystallizer_2_3(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2a'] <= model.batch_product[i, 'alice']


model.constr11 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_2_3)


def constraint_output_from_crystallizer_3_1(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2b'] >= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_1'] + \
           model.batch_product[i, 'bob'] - 1


model.constr12 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_3_1)


def constraint_output_from_crystallizer_3_2(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2b'] <= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_1']


model.constr13 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_3_2)


def constraint_output_from_crystallizer_3_3(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_1', 'out_2b'] <= model.batch_product[i, 'bob']


model.constr14 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_3_3)


def constraint_output_from_crystallizer_4_1(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2b'] >= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_2'] + \
           model.batch_product[i, 'bob'] - 1


model.constr15 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_4_1)


def constraint_output_from_crystallizer_4_2(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2b'] <= model.matrix_recipe[i, 'pasteurizer',
                                                                                             'crystallization_tank_2']


model.constr16 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_4_2)


def constraint_output_from_crystallizer_4_3(model, i):
    return model.matrix_recipe[i, 'crystallization_tank_2', 'out_2b'] <= model.batch_product[i, 'bob']


model.constr17 = pyomo.environ.Constraint(model.total_batch_index, rule=constraint_output_from_crystallizer_4_3)

model.batch_start = pyomo.environ.Var(model.total_batch_index, model.machines, model.machines,
                                      within=pyomo.environ.NonNegativeReals)

model.machine_pair_durations = pyomo.environ.Param(model.machines, model.machines,
                                                   initialize=machine_pair_durations)

model.machine_process_durations = pyomo.environ.Param(model.machines, initialize=machine_process_durations)


def constraint_starting_times_1(model, i, i_prime, m, n, l, p):
    if i_prime < i:
        return model.batch_start[i, m, n] >= model.batch_start[i_prime, l, m] + model.matrix_recipe[i_prime, l, m] * \
               (model.machine_pair_durations[l, m] + model.machine_process_durations[m] + \
                model.machine_pair_durations[m, p]) + (model.matrix_recipe[i, m, n] - 1) * 10e9
    else:
        return pyomo.environ.Constraint.Skip


model.constr18 = pyomo.environ.Constraint(model.total_batch_index, model.total_batch_index, model.machines,
                                          model.machines, model.machines, model.machines,
                                          rule=constraint_starting_times_1)


def constraint_starting_times_2(model, i, i_prime, m, n, l, p):
    if i_prime < i:
        return model.batch_start[i, m, n] >= model.batch_start[i_prime, l, n] + model.matrix_recipe[i_prime, l, n] * \
               (model.machine_pair_durations[l, n] + model.machine_process_durations[n] + \
                model.machine_pair_durations[n, p]) + (model.matrix_recipe[i, m, n] - 1) * 10e9
    else:
        return pyomo.environ.Constraint.Skip


model.constr19 = pyomo.environ.Constraint(model.total_batch_index, model.total_batch_index, model.machines,
                                          model.machines, model.machines, model.machines,
                                          rule=constraint_starting_times_2)

model.is_batch_process = pyomo.environ.Param(model.machines, model.machines, initialize=is_batch_process)


def constraint_starting_times_3(model, i, m, n, l):
    return model.batch_start[i, m, n] >= model.batch_start[i, l, m] + model.is_batch_process[l, m] * \
           (model.machine_pair_durations[l, m] + model.machine_process_durations[l]) + \
           (model.matrix_recipe[i, m, n] - 1) * 10e9


model.constr20 = pyomo.environ.Constraint(model.total_batch_index, model.machines,
                                          model.machines, model.machines, rule=constraint_starting_times_3)


def constraint_starting_times_4(model, i, m, n, l):
    if model.is_batch_process[m, n] == 1:
        return model.batch_start[i, m, n] <= model.batch_start[i, l, m] + (1 - model.matrix_recipe[i, m, n]) * 10e9
    else:
        return pyomo.environ.Constraint.Skip


model.constr21 = pyomo.environ.Constraint(model.total_batch_index, model.machines,
                                          model.machines, model.machines, rule=constraint_starting_times_4)

"""
Cuidado con esta última restricción, podría causar bugs en el futuro
"""

model.aux_objective = pyomo.environ.Var(within=pyomo.environ.NonNegativeReals)


def constraint_min_max(model, i, m, n):
    return model.aux_objective >= model.batch_start[i, m, n]


model.constr22 = pyomo.environ.Constraint(model.total_batch_index, model.machines, model.machines,
                                          rule=constraint_min_max)


def objective_function(model):
    return model.aux_objective


model.obj = pyomo.environ.Objective(rule=objective_function)

"""
Resoluciópn del modelo
"""
optimizer = pyomo.environ.SolverFactory("cbc")
optimizer.solve(model)

for v in model.component_objects(pyomo.environ.Var, active=True):
    print("Variable component object", v)
    print("Type of component object: ", str(type(v))[1:-1])
    varobject = getattr(model, str(v))
    print("Type of object accessed via getattr: ", str(type(varobject))[1:-1])
    for index in varobject:
        print("   ", index, varobject[index].value)

"""
Creación de un pandas.DataFrame con el horario obtenido de la resolución del modelo
"""
results = [{'Batch Number': i,
            'Start': model.batch_start[i, m, n].value / 3600,
            'Duration': (model.matrix_recipe[i, m, n].value * (model.machine_pair_durations[m, n] + \
                                                               model.machine_process_durations[m])) / 3600,
            'Finish': (model.batch_start[i, m, n].value + model.matrix_recipe[i, m, n].value * \
                       (model.machine_pair_durations[m, n] + model.machine_process_durations[m])) / 3600,
            'Machine 1': m,
            'Machine 2': n,
            'Name': str(m) + '->' + str(n)}
           for i in model.total_batch_index for m in model.machines for n in model.machines]

print(results)

"""
Creación de un Gantt Chart para la representación gráfica del horario obtenido por el modelo
"""

pandas.options.mode.chained_assignment = None

schedule = pandas.DataFrame(results)
schedule_red = schedule[schedule['Duration'] > 0]
for i in schedule_red.index:
    if machine_process_durations[schedule_red.loc[i, 'Machine 1']] > 0:
        schedule_red.loc[i - 1] = [schedule_red.loc[i, 'Batch Number'],
                                   schedule_red.loc[i, 'Start'],
                                   machine_process_durations[schedule_red.loc[i, 'Machine 1']] / 3600,
                                   schedule_red.loc[i, 'Start'] + \
                                   machine_process_durations[schedule_red.loc[i, 'Machine 1']] / 3600,
                                   schedule_red.loc[i, 'Machine 1'],
                                   '',
                                   schedule_red.loc[i, 'Machine 1']]
        schedule_red.loc[i, 'Start'] = schedule_red.loc[i, 'Start'] + \
                                       machine_process_durations[schedule_red.loc[i, 'Machine 1']] / 3600
        schedule_red.loc[i, 'Duration'] = schedule_red.loc[i, 'Duration'] - \
                                          machine_process_durations[schedule_red.loc[i, 'Machine 1']] / 3600

schedule_red = schedule_red.sort_index().reset_index(drop=True)
print(schedule_red)


def visualize(schedule):
    batches = sorted(list(schedule['Batch Number'].unique()))
    machines = available_machines

    bar_style = {'alpha': 1.0, 'lw': 25, 'solid_capstyle': 'butt'}
    text_style = {'color': 'white', 'weight': 'bold', 'ha': 'center', 'va': 'center'}
    colors = matplotlib.cm.Dark2.colors

    fig, ax = matplotlib.pyplot.subplots(2, 1, figsize=(12, 5 + (len(batches) + len(machines)) / 4))

    for i in schedule.index:
        xs = schedule.loc[i, 'Start']
        xf = schedule.loc[i, 'Finish']
        ax[0].plot([xs, xf], [schedule.loc[i, 'Batch Number']] * 2,
                   c=colors[schedule.loc[i, 'Batch Number'] % len(batches)], **bar_style)
        ax[1].plot([xs, xf], [schedule.loc[i, 'Machine 1']] * 2,
                   c=colors[schedule.loc[i, 'Batch Number'] % len(batches)], **bar_style)
        ax[1].text((xs + xf) / 2, schedule.loc[i, 'Machine 1'], schedule.loc[i, 'Name'], **text_style)
        if schedule.loc[i, 'Machine 2'] != "":
            ax[1].plot([xs, xf], [schedule.loc[i, 'Machine 2']] * 2,
                       c=colors[schedule.loc[i, 'Batch Number'] % len(batches)], **bar_style)
            ax[1].text((xs + xf) / 2, schedule.loc[i, 'Machine 2'], schedule.loc[i, 'Name'], **text_style)

    ax[0].set_title('Batch Schedule')
    ax[0].set_ylabel('Batches')
    ax[1].set_title('Machine Schedule')
    ax[1].set_ylabel('Machine')

    fig.tight_layout()
    matplotlib.pyplot.show()


visualize(schedule_red)
