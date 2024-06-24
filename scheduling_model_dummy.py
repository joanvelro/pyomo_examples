"""
Script de un modelo muy simplificado del MVP de GEA como primer paso para la implementación del modelo total
"""

import pyomo.environ
import numpy
import pandas
import matplotlib.pyplot

"""
Creación del modelo de pyomo
"""
dummy_model = pyomo.environ.ConcreteModel()
"""
Definición de los parámetros dentro del objeto modelo
"""
dummy_model.order_kg = pyomo.environ.Param(initialize=4.3e5)
dummy_model.number_of_days = pyomo.environ.Param(initialize=2)
dummy_model.start_of_work_time_s = pyomo.environ.Param(initialize=8 * 3600)
dummy_model.duration_of_work_day_s = pyomo.environ.Param(initialize=15 * 3600)
dummy_model.consumption_kw_s = pyomo.environ.Param(initialize=30 / 3600)
dummy_model.transfer_line_flow_rate_kg_per_s = pyomo.environ.Param(initialize=20000 / 3600)
dummy_model.maximum_number_transfer_lines = pyomo.environ.Param(initialize=20)
dummy_model.maximum_continuous_production_time_pasteurizer_s = pyomo.environ.Param(initialize=18000)
dummy_model.maximum_batches_between_cleanings = pyomo.environ.Param(initialize=2)
dummy_model.cleaning_duration_s = pyomo.environ.Param(initialize=3000)
dummy_model.maximum_number_batches = pyomo.environ.Param(initialize=numpy.ceil(dummy_model.order_kg / (
        dummy_model.transfer_line_flow_rate_kg_per_s * dummy_model.maximum_continuous_production_time_pasteurizer_s)))

"""
Definición de las variables dentro del objeto modelo
"""
dummy_model.batch_index = pyomo.environ.Set(
    initialize=list(range(1, int(dummy_model.maximum_number_batches.value) + 1)))


def init_inverse_number_transfer_lines(m):
    return [1 / i for i in range(1, int(m.maximum_number_transfer_lines) + 1)]


dummy_model.possible_values_inverse_number_transfer_lines = pyomo.environ.Set(
    initialize=init_inverse_number_transfer_lines)

dummy_model.select_possible_values_inverse_number_transfer_lines = pyomo.environ.Var(
    dummy_model.possible_values_inverse_number_transfer_lines, domain=pyomo.environ.Binary)
dummy_model.inverse_number_transfer_lines = pyomo.environ.Var(within=pyomo.environ.Reals)


def pick_one_value(m):
    return 1 == sum(m.select_possible_values_inverse_number_transfer_lines[i] for i in
                    m.possible_values_inverse_number_transfer_lines)


dummy_model.auxconstr1 = pyomo.environ.Constraint(rule=pick_one_value)


def set_inverse_number_transfer_lines(m):
    return m.inverse_number_transfer_lines == sum(i * m.select_possible_values_inverse_number_transfer_lines[i] for i in
                                                  m.possible_values_inverse_number_transfer_lines)


dummy_model.auxconstr2 = pyomo.environ.Constraint(rule=set_inverse_number_transfer_lines)

dummy_model.number_batches = pyomo.environ.Var(within=pyomo.environ.PositiveIntegers)

dummy_model.batch_start_time_s = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.NonNegativeReals)

dummy_model.batch_duration_time_s = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.NonNegativeReals)

dummy_model.batch_duration_aux = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.NonNegativeReals)

dummy_model.existing_batches = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.Binary)

dummy_model.incomplete_batches = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.Binary)

dummy_model.objective_function_aux = pyomo.environ.Var(dummy_model.possible_values_inverse_number_transfer_lines,
                                                       within=pyomo.environ.NonNegativeReals)

dummy_model.total_working_time_s = pyomo.environ.Var(within=pyomo.environ.NonNegativeReals)

dummy_model.cleaning_indicator = pyomo.environ.Var(dummy_model.batch_index, within=pyomo.environ.Binary)

dummy_model.cleaning_indicator_divisibility_check = pyomo.environ.Var(dummy_model.batch_index,
                                                                      within=pyomo.environ.Binary)

dummy_model.cleaning_indicator_aux = pyomo.environ.Var(dummy_model.batch_index,
                                                       within=pyomo.environ.NonNegativeIntegers)

dummy_model.cleaning_indicator_remainder = pyomo.environ.Var(dummy_model.batch_index,
                                                             within=pyomo.environ.NonNegativeReals)

dummy_model.cleaning_indicator_partial_sums = pyomo.environ.Var(dummy_model.batch_index,
                                                                within=pyomo.environ.NonNegativeIntegers)

dummy_model.assigned_day_of_operation = pyomo.environ.Var(dummy_model.batch_index,
                                                          within=pyomo.environ.PositiveIntegers,
                                                          bounds=(1, dummy_model.number_of_days))
"""
Definición de la función objetivo
"""


def objective_function(dummy_model):
    return sum(dummy_model.objective_function_aux[j] for j in dummy_model.possible_values_inverse_number_transfer_lines) \
           - dummy_model.inverse_number_transfer_lines


dummy_model.obj = pyomo.environ.Objective(rule=objective_function)
"""
Definición de las restricciones
"""


def constraint_number_batches_lower(dummy_model):
    return dummy_model.order_kg * dummy_model.inverse_number_transfer_lines / (
                dummy_model.transfer_line_flow_rate_kg_per_s * \
                dummy_model.maximum_continuous_production_time_pasteurizer_s) <= dummy_model.number_batches


dummy_model.constr1 = pyomo.environ.Constraint(rule=constraint_number_batches_lower)


def constraint_number_batches_upper(dummy_model):
    return dummy_model.order_kg * dummy_model.inverse_number_transfer_lines / (
                dummy_model.transfer_line_flow_rate_kg_per_s * \
                dummy_model.maximum_continuous_production_time_pasteurizer_s) + 0.999 >= dummy_model.number_batches


dummy_model.constr2 = pyomo.environ.Constraint(rule=constraint_number_batches_upper)


def constraint_starting_times(dummy_model, i):
    if i == 1:
        return dummy_model.batch_start_time_s[i] == dummy_model.start_of_work_time_s
    else:
        return dummy_model.batch_start_time_s[i - 1] + dummy_model.batch_duration_time_s[i - 1] + \
               dummy_model.cleaning_indicator[i - 1] * dummy_model.cleaning_duration_s <= \
               dummy_model.batch_start_time_s[i]


dummy_model.constr3 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_starting_times)


def constraint_existing_batches_lower(dummy_model):
    return sum(dummy_model.existing_batches[i] for i in dummy_model.batch_index) == dummy_model.number_batches


dummy_model.constr4 = pyomo.environ.Constraint(rule=constraint_existing_batches_lower)


def constraint_incomplete_batches(dummy_model):
    return sum(dummy_model.incomplete_batches[i] for i in dummy_model.batch_index) == 1


dummy_model.constr5 = pyomo.environ.Constraint(rule=constraint_incomplete_batches)


def constraint_incomplete_batches_aux(dummy_model, i):
    return dummy_model.existing_batches[i] - dummy_model.incomplete_batches[i] >= 0


dummy_model.constr6 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_incomplete_batches_aux)


def constraint_batch_duration_aux_1(dummy_model, i):
    return dummy_model.batch_duration_aux[i] <= dummy_model.incomplete_batches[i] * 10e9


dummy_model.constr7 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_batch_duration_aux_1)


def constraint_batch_duration_aux_2(dummy_model, i):
    return dummy_model.batch_duration_aux[
               i] >= dummy_model.number_batches * dummy_model.maximum_continuous_production_time_pasteurizer_s - \
           dummy_model.order_kg * \
           dummy_model.inverse_number_transfer_lines / dummy_model.transfer_line_flow_rate_kg_per_s - \
           (1 - dummy_model.incomplete_batches[i]) * 10e9


dummy_model.constr8 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_batch_duration_aux_2)


def constraint_batch_duration_aux_3(dummy_model, i):
    return dummy_model.batch_duration_aux[
               i] <= dummy_model.number_batches * dummy_model.maximum_continuous_production_time_pasteurizer_s - \
           dummy_model.order_kg \
           * dummy_model.inverse_number_transfer_lines / dummy_model.transfer_line_flow_rate_kg_per_s


dummy_model.constr9 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_batch_duration_aux_3)


def constraint_batch_duration(dummy_model, i):
    return dummy_model.batch_duration_time_s[i] == dummy_model.existing_batches[i] * \
           dummy_model.maximum_continuous_production_time_pasteurizer_s - dummy_model.batch_duration_aux[i]


dummy_model.constr10 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_batch_duration)


def constraint_time_horizon(dummy_model, i):
    return dummy_model.batch_start_time_s[i] + \
           dummy_model.batch_duration_time_s[i] + dummy_model.cleaning_indicator[i] * \
           dummy_model.cleaning_duration_s <= dummy_model.start_of_work_time_s + 24 * 3600 * \
           (dummy_model.assigned_day_of_operation[i] - 1) + dummy_model.duration_of_work_day_s


dummy_model.constr11 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_time_horizon)


def constraint_start_of_work_times(dummy_model, i):
    return dummy_model.batch_start_time_s[i] >= dummy_model.start_of_work_time_s + 24 * 3600 * \
           (dummy_model.assigned_day_of_operation[i] - 1)


dummy_model.constr12 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_start_of_work_times)


def constraint_total_working_time(dummy_model):
    return sum(
        dummy_model.batch_duration_time_s[i] for i in dummy_model.batch_index) == dummy_model.total_working_time_s


dummy_model.constr13 = pyomo.environ.Constraint(rule=constraint_total_working_time)


def constraint_objective_function_aux_1(dummy_model, j):
    return dummy_model.objective_function_aux[j] <= dummy_model.select_possible_values_inverse_number_transfer_lines[
        j] * 10e9


dummy_model.constr14 = pyomo.environ.Constraint(dummy_model.possible_values_inverse_number_transfer_lines,
                                                rule=constraint_objective_function_aux_1)


def constraint_objective_function_aux_2(dummy_model, j):
    return dummy_model.objective_function_aux[j] >= dummy_model.total_working_time_s / j - \
           (1 - dummy_model.select_possible_values_inverse_number_transfer_lines[j]) * 10e9


dummy_model.constr15 = pyomo.environ.Constraint(dummy_model.possible_values_inverse_number_transfer_lines,
                                                rule=constraint_objective_function_aux_2)


def constraint_objective_function_aux_3(dummy_model, j):
    return dummy_model.objective_function_aux[j] <= dummy_model.total_working_time_s / j


dummy_model.constr16 = pyomo.environ.Constraint(dummy_model.possible_values_inverse_number_transfer_lines,
                                                rule=constraint_objective_function_aux_3)


def constraint_partial_sums(dummy_model, i):
    return dummy_model.cleaning_indicator_partial_sums[i] == \
           sum(dummy_model.existing_batches[i_prime] for i_prime in list(range(1, i + 1)))


dummy_model.constr17 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_partial_sums)


def constraint_cleaning_modulo_1(dummy_model, i):
    return dummy_model.cleaning_indicator_aux[i] * dummy_model.maximum_batches_between_cleanings + \
           dummy_model.cleaning_indicator_remainder[i] == dummy_model.cleaning_indicator_partial_sums[i]


dummy_model.constr18 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_cleaning_modulo_1)


def constraint_cleaning_modulo_2(dummy_model, i):
    return dummy_model.cleaning_indicator_remainder[i] <= dummy_model.maximum_batches_between_cleanings - 0.001


dummy_model.constr19 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_cleaning_modulo_2)


def constraint_cleaning_divisibility_binary_upper(dummy_model, i):
    return dummy_model.cleaning_indicator_divisibility_check[i] <= 1 - dummy_model.cleaning_indicator_remainder[
        i] / 10e4


dummy_model.constr20 = pyomo.environ.Constraint(dummy_model.batch_index,
                                                rule=constraint_cleaning_divisibility_binary_upper)


def constraint_cleaning_divisibility_binary_lower(dummy_model, i):
    return dummy_model.cleaning_indicator_divisibility_check[i] >= - dummy_model.cleaning_indicator_remainder[i] + 0.001


dummy_model.constr21 = pyomo.environ.Constraint(dummy_model.batch_index,
                                                rule=constraint_cleaning_divisibility_binary_lower)


def constraint_cleaning_indicator_upper(dummy_model, i):
    return dummy_model.cleaning_indicator[i] <= dummy_model.cleaning_indicator_divisibility_check[i] + \
           dummy_model.existing_batches[i] - 1


dummy_model.constr22 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_cleaning_indicator_upper)


def constraint_cleaning_indicator_lower(dummy_model, i):
    return dummy_model.cleaning_indicator[i] >= dummy_model.cleaning_indicator_divisibility_check[i] + \
           dummy_model.existing_batches[i] - 1.5


dummy_model.constr23 = pyomo.environ.Constraint(dummy_model.batch_index, rule=constraint_cleaning_indicator_lower)
"""
Resoluciópn del modelo
"""
optimizer = pyomo.environ.SolverFactory("cbc")
optimizer.solve(dummy_model)
"""
Impresión en pantalla de las variables del modelo
"""
for v in dummy_model.component_objects(pyomo.environ.Var, active=True):
    print("Variable component object", v)
    print("Type of component object: ", str(type(v))[1:-1])
    varobject = getattr(dummy_model, str(v))
    print("Type of object accessed via getattr: ", str(type(varobject))[1:-1])
    for index in varobject:
        print("   ", index, varobject[index].value)

"""
Creación de un pandas.DataFrame con el horario obtenido de la resolución del modelo
"""
results = [{'Batch Number': dummy_model.batch_index[i],
            'Start': dummy_model.batch_start_time_s[i].value / 3600,
            'Duration': (dummy_model.batch_duration_time_s[i].value + dummy_model.cleaning_indicator[i].value *
                         dummy_model.cleaning_duration_s.value) / 3600,
            'Finish': (dummy_model.batch_start_time_s[i].value + dummy_model.batch_duration_time_s[i].value +
                       dummy_model.cleaning_indicator[i].value * dummy_model.cleaning_duration_s.value) / 3600,
            'Cleaning Included': dummy_model.cleaning_indicator[i].value}
           for i in dummy_model.batch_index]

print(results)
"""
Creación de un Gantt Chart para la representación gráfica del horario obtenido por el modelo
"""


def visualize(results):
    schedule = pandas.DataFrame(results)

    bar_style = {'alpha': 1.0, 'lw': 35, 'solid_capstyle': 'butt'}
    text_style = {'color': 'black', 'weight': 'bold', 'ha': 'center', 'va': 'center'}
    colors = ['cornflowerblue', 'royalblue']

    schedule.sort_values(by=['Batch Number', 'Start'])
    schedule.set_index(['Batch Number'], inplace=True)

    fig = matplotlib.pyplot.figure()
    ax = matplotlib.pyplot.gca()

    ax.set_xlim([0, 24 * dummy_model.number_of_days.value])
    ax.set_ylim([0, 3])

    k = 1
    for j in schedule.index:
        if schedule.loc[j, 'Cleaning Included'] < 0.5:
            xs = schedule.loc[j, 'Start']
            xf = schedule.loc[j, 'Finish']
            ax.plot([xs, xf], [1, 1], c=colors[j % len(colors)], **bar_style)
            ax.text((xs + xf) / 2, 1, 'Batch ' + str(schedule.index[j - 1]), **text_style)
        else:
            xs = schedule.loc[j, 'Start']
            xf = schedule.loc[j, 'Finish'] - dummy_model.cleaning_duration_s / 3600
            ax.plot([xs, xf], [1, 1], c=colors[j % len(colors)], **bar_style)
            ax.text((xs + xf) / 2, 1, 'Batch ' + str(schedule.index[j - 1]), **text_style)
            cs = schedule.loc[j, 'Finish'] - dummy_model.cleaning_duration_s / 3600
            cf = schedule.loc[j, 'Finish']
            ax.plot([cs, cf], [2, 2], c='firebrick', **bar_style)
            ax.text((cs + cf) / 2, 2, 'Cleaning ' + str(k), **text_style)
            k = k + 1
    inicios = [8 + 24 * i for i in range(int(dummy_model.number_of_days))]
    finales = [8 + 15 + 24 * i for i in range(int(dummy_model.number_of_days))]
    ax.vlines(inicios, 0, 3, color='darkgreen', linestyle='-.')
    ax.vlines(finales, 0, 3, color='firebrick', linestyle='-.')
    ax.set_title(
        'Job Schedule; Number of transfer lines equal to : ' + str(1 / dummy_model.inverse_number_transfer_lines.value))
    ax.set_ylabel('Process')
    ax.set_yticks(range(1, int(dummy_model.number_of_days) + 1))
    ax.set_yticklabels(['Pasteurizer', 'Cleanings'])
    ax.grid(True)

    fig.tight_layout()
    matplotlib.pyplot.show()


visualize(results)
