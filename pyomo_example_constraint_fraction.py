import pyomo.environ as pyo
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition

# model
model = pyo.ConcreteModel()

model.param_fuel_purchase = {'diesel': 12000}
model.set_fuels = ['diesel']
model.set_fuel_types = ['terrestre']
model.set_bios = ['fame', 'hvo']
model.set_centers = ['Center_A', 'Center_B']
model.set_suppliers = ['Supplier_1', 'Supplier_2']

model.param_max_bio_volume = {'fame': 1200, 'hvo': 1200}
model.param_min_bio_volume = {'fame': 100, 'hvo': 100}

model.var_bio_purchase = pyo.Var(model.set_fuels,
                                 model.set_centers,
                                 model.set_suppliers,
                                 model.set_fuel_types,
                                 model.set_bios,
                                 domain=pyo.NonNegativeIntegers, initialize=0)

model.param_fuel_energy_conversion = {'diesel': 0.87}
model.param_bio_energy_conversion = {'hvo': 0.67, 'fame': .55}
model.param_bio_dc = {'hvo': 0.89, 'fame': 0.32}
model.param_bio_cost = {'hvo': 120, 'fame': 180}
Min_ec = 0.11


def _exp_bio_energy_content(model, fuel_type):
    return sum(model.var_bio_purchase[fuel, center, supplier, fuel_type, bio] *
               model.param_bio_energy_conversion[bio]
               for fuel in model.set_fuels
               for center in model.set_centers
               for supplier in model.set_suppliers
               for bio in model.set_bios)


model.exp_bio_energy_content = pyo.Expression(model.set_fuel_types, rule=_exp_bio_energy_content)


def _exp_bio_energy_content_dc(model, fuel_type):
    return sum(model.var_bio_purchase[fuel, center, supplier, fuel_type, bio] *
               model.param_bio_energy_conversion[bio] * model.param_bio_dc[bio]
               for fuel in model.set_fuels
               for center in model.set_centers
               for supplier in model.set_suppliers
               for bio in model.set_bios)


model.exp_bio_energy_content_dc = pyo.Expression(model.set_fuel_types, rule=_exp_bio_energy_content_dc)


def _exp_fuel_energy_content(model, fuel_type):
    return sum(
        (model.param_fuel_purchase[fuel] -
         sum(model.var_bio_purchase[fuel, center, supplier, fuel_type, bio]
             for center in model.set_centers
             for supplier in model.set_suppliers
             for bio in model.set_bios)
         ) * model.param_fuel_energy_conversion[fuel] for fuel in model.set_fuels)


model.exp_fuel_energy_content = pyo.Expression(model.set_fuel_types, rule=_exp_fuel_energy_content)


def _constraint_energy_content(model, fuel_type):
    return (model.exp_bio_energy_content[fuel_type] + model.exp_bio_energy_content_dc[fuel_type]) \
        >= Min_ec * (model.exp_fuel_energy_content[fuel_type] + model.exp_bio_energy_content[fuel_type])


model.constraint_energy_content = pyo.Constraint(model.set_fuel_types, rule=_constraint_energy_content)

def _constraint_max_volume_purchase(model, bio, fuel_type):
    bio_quantity = sum(model.var_bio_purchase[fuel, center, supplier, fuel_type, bio]
               for fuel in model.set_fuels
               for center in model.set_centers
               for supplier in model.set_suppliers)

    return bio_quantity <= model.param_max_bio_volume[bio]


model.constraint_max_volume_purchase = pyo.Constraint(model.set_bios, model.set_fuel_types, rule=_constraint_max_volume_purchase)


def _constraint_min_volume_purchase(model, bio, fuel_type):
    bio_quantity = sum(model.var_bio_purchase[fuel, center, supplier, fuel_type, bio]
               for fuel in model.set_fuels
               for center in model.set_centers
               for supplier in model.set_suppliers)

    return bio_quantity >= model.param_min_bio_volume[bio]


model.constraint_min_volume_purchase = pyo.Constraint(model.set_bios, model.set_fuel_types,
                                                      rule=_constraint_min_volume_purchase)


def _objective_function(model):
    return sum(model.param_bio_cost[bio] * model.var_bio_purchase[fuel, center, supplier, fuel_type, bio]
               for fuel in model.set_fuels
               for center in model.set_centers
               for supplier in model.set_suppliers
               for fuel_type in model.set_fuel_types
               for bio in model.set_bios)


model.objective_function = pyo.Objective(rule=_objective_function, sense=pyo.minimize, doc='Minimize cost profit')


def _exp_energy_content(model, fuel_type):
    return (model.exp_bio_energy_content[fuel_type] + model.exp_bio_energy_content_dc[fuel_type]) / (
                model.exp_fuel_energy_content[fuel_type] + model.exp_bio_energy_content[fuel_type])


model.exp_energy_content = pyo.Expression(model.set_fuel_types, rule=_exp_energy_content)


print('\n')
print('\n')
print('--- Model Summary ---')
print(model.pprint())

print('\n')
print('\n')
print('--- Model Solver ---')
opt = SolverFactory("cbc")  # glpk
results = opt.solve(model, tee=True)  # tee=True for details

print('\n')
print('\n')
print('--- Model Results ---')

# Check the solution

if (results.solver.status == SolverStatus.ok) and \
        (results.solver.termination_condition == TerminationCondition.optimal):
    print('Optimal Solution')

    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())


    for (key, var) in model.var_bio_purchase.items():
        print('var_bio_purchase({}):{}'.format(key, pyo.value(var)))

    for (key, var) in model.objective_function.items():
        print('objective_function({}):{}'.format(key, pyo.value(var)))

    for (key, var) in model.exp_bio_energy_content.items():
        print('exp_bio_energy_content({}):{}'.format(key, pyo.value(var)))

    for (key, var) in model.exp_bio_energy_content_dc.items():
        print('exp_bio_energy_content_dc({}):{}'.format(key, pyo.value(var)))

    for (key, var) in model.exp_fuel_energy_content.items():
        print('exp_fuel_energy_content({}):{}'.format(key, pyo.value(var)))

    for (fuel_type, var) in model.exp_energy_content.items():
        print('exp_energy_content({}):{}'.format(fuel_type, pyo.value(var)))


    # Do something when the solution in optimal and feasible

elif results.solver.termination_condition == TerminationCondition.infeasible:
    print('(!) Infeasible solution')
else:
    print('(!)')
    print('Status:', results.solver.status)
    # Something else is wrong
    print('termination condition:', results.solver.termination_condition)
