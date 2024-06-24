import logging

from pyomo.util.infeasible import log_infeasible_constraints
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import pyomo.environ as pyo

logger = logging.getLogger(__name__)


def has_solution(results):
    """
    Check the solution
    """
    status = results.solver.status
    condition = results.solver.termination_condition

    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    if not solution:
        logger.error(results)
        logger.error('Something is wrong -> status {s} and condition {c}'.format(s=status, c=condition))
        if condition == pyo.TerminationCondition.infeasible:
            log_infeasible_constraints(model,
                                       logger=logger,
                                       log_variables=True,
                                       log_expression=True)

    if (results.solver.status == SolverStatus.ok) and (
            results.solver.termination_condition == TerminationCondition.optimal):
        print(':::Feasible solution:::')

        print('--- Problem Results ---')
        print(results.Problem())
        print('--- Solver Results ---')
        print(results.Solver())

    elif results.solver.termination_condition == TerminationCondition.infeasible:
        print('(!) Infeasible solution')
    else:
        print('(!)')
        print('Status:', results.solver.status)
        # Something else is wrong
        print('termination condition:', results.solver.termination_condition)


periods = [1, 2, 3, 4, 5, 6, 7]

# Datos del problema
products = ['Producto1',
            'Producto2']

nodos = ['Origen1', 'Origen2', 'Destino1', 'Destino2', 'Almacen1', 'Almacen2']

demands = {(3, 'Producto1', 'Destino2'): 43,
           (5, 'Producto2', 'Destino1'): 23,
           (6, 'Producto2', 'Destino1'): 13,
           (7, 'Producto1', 'Destino1'): 53,
           (4, 'Producto1', 'Destino2'): 33,
           (5, 'Producto2', 'Destino1'): 43}

factories = {'Origen1': 1000,
             'Origen2': 1000}

destinations = ['Destino1',
                'Destino2']

warehouses = {'Almacen1': 200,
              'Almacen2': 300}

logistic_cost = {
    'Origen1': {'Destino1': 5, 'Destino2': 8, 'Almacen1': 2, 'Almacen2': 3},
    'Origen2': {'Destino1': 4, 'Destino2': 6, 'Almacen1': 3, 'Almacen2': 5},
    'Almacen1': {'Destino1': 1, 'Destino2': 2, 'Origen1': 0, 'Origen2': 0},
    'Almacen2': {'Destino1': 2, 'Destino2': 3, 'Origen1': 0, 'Origen2': 0},
    'Destino1': {'Origen1': 0, 'Origen2': 0, 'Almacen1': 0, 'Almacen2': 0, 'Destino2': 0},
    'Destino2': {'Origen1': 0, 'Origen2': 0, 'Almacen1': 0, 'Almacen2': 0, 'Destino1': 0},
}
travel_days = {
    'Origen1': {'Destino1': 1, 'Destino2': 2, 'Almacen1': 1, 'Almacen2': 1, 'Origen2': 1e9},
    'Origen2': {'Destino1': 2, 'Destino2': 2, 'Almacen1': 3, 'Almacen2': 2, 'Origen1': 1e9},
    'Almacen1': {'Destino1': 2, 'Destino2': 3, 'Origen1': 0, 'Origen2': 0, 'Almacen2': 1e9},
    'Almacen2': {'Destino1': 1, 'Destino2': 2, 'Origen1': 0, 'Origen2': 0, 'Almacen1': 1e9},
    'Destino1': {'Origen1': 1e9, 'Origen2': 1e9, 'Almacen1': 1e9, 'Almacen2': 1e9, 'Destino2': 1e9},
    'Destino2': {'Origen1': 1e9, 'Origen2': 1e9, 'Almacen1': 1e9, 'Almacen2': 1e9, 'Destino1': 1e9},
}

# Definir el modelo
model = pyo.ConcreteModel()

# Definir índices
model.set_periods = pyo.Set(initialize=periods)
model.set_products = pyo.Set(initialize=products)
model.set_factories = pyo.Set(initialize=factories.keys())
model.set_warehouses = pyo.Set(initialize=warehouses.keys())
model.set_destinations = pyo.Set(initialize=destinations)
model.set_destinations_warehouses = pyo.Set(initialize=list(destinations + list(warehouses.keys())))

# [product, factory, destination/warehouse]
model.var_movement_factory = pyo.Var(model.set_periods,
                                     model.set_products,
                                     model.set_factories,
                                     model.set_destinations_warehouses,
                                     domain=pyo.NonNegativeReals,
                                     initialize=0)

# [product, warehouse, destination]
model.var_movement_warehouse = pyo.Var(
    model.set_periods,
    model.set_products,
    model.set_warehouses,
    model.set_destinations,
    domain=pyo.NonNegativeReals,
    initialize=0)

model.slack_demand = pyo.Var(
    model.set_periods,
    model.set_products,
    model.set_destinations,
    domain=pyo.NonNegativeReals,
    initialize=0
    )


def objective_function(model):
    total_cost_factory = sum(
        logistic_cost[origin][destination] * model.var_movement_factory[period, product, origin, destination]
        for period in model.set_periods
        for product in model.set_products
        for origin in model.set_factories
        for destination in model.set_destinations_warehouses
    )
    total_cost_warehouse = sum(
        logistic_cost[origin][destination] * model.var_movement_warehouse[period, product, origin, destination]
        for period in model.set_periods
        for product in model.set_products
        for origin in model.set_warehouses
        for destination in model.set_destinations
    )
    slack_demand = sum(
        model.slack_demand[period, product, destination] * 1e3
        for period in model.set_periods
        for product in model.set_products
        for destination in model.set_destinations
    )
    return total_cost_factory + total_cost_warehouse + slack_demand


# Definir la función objetivo
model.objetive = pyo.Objective(rule=objective_function, sense=pyo.minimize)


# Restricciones de factories
def factories_production_limit(model, factory, period):
    return sum(model.var_movement_factory[period, product, factory, destination]
               for product in model.set_products
               for destination in model.set_destinations_warehouses) <= factories[factory]


model.factories_production_limit = pyo.Constraint(model.set_factories, model.set_periods,
                                             rule=factories_production_limit)


def demand_constraint(model, period_demand, product, destination):
    sent_from_factory = sum(model.var_movement_factory[period, product, factory, destination]
                            for period in model.set_periods
                            for product in model.set_products
                            for factory in model.set_factories
                            if period_demand - period >= travel_days[factory][destination]
                            )
    sent_from_warehouse = sum(model.var_movement_warehouse[period, product, warehouse, destination]
                              for period in model.set_periods
                              for product in model.set_products
                              for warehouse in model.set_warehouses
                              if period_demand - period >= travel_days[warehouse][destination]
                              )
    slack = model.slack_demand[period_demand, product, destination]
    demand = demands[(period_demand, product, destination)]
    return sent_from_factory + sent_from_warehouse + slack == demand


model.demand_constraint = pyo.Constraint(demands.keys(), rule=demand_constraint)


def stock_warehouse(model, period, product, warehouse):
    if period == 1:
        return warehouses[warehouse]
    else:
        total_in = sum(model.var_movement_factory[period, product, factory, warehouse]
                       for factory in model.set_factories)
        total_out = sum(model.var_movement_warehouse[period, product, warehouse, destination]
                        for destination in model.set_destinations)
        return model.stock_warehouse[period - 1, product, warehouse] - total_out + total_in


model.stock_warehouse = pyo.Expression(model.set_periods, model.set_products, warehouses.keys(),
                                       rule=stock_warehouse)


# Restricciones de capacidad de almacenamiento
def warehouses_constraint(model, period, product, warehouse):
    return model.stock_warehouse[period, product, warehouse] >= 0


model.warehouses_constraint = pyo.Constraint(model.set_periods, model.set_products, warehouses.keys(),
                                       rule=warehouses_constraint)

# Resolver el modelo
solver = 'cbc'
optimizer = SolverFactory(solver)
results = optimizer.solve(model, tee=True)

# Check the solution
if has_solution(results):
    logger.info('\n')
    for ((period, product, warehouse, destination), var) in model.var_movement_warehouse.items():
        value_ = pyo.value(var)
        if value_ != 0:
            logger.info(
                'var_movement_warehouse({},{},{},{}):{}'.format(period, product, warehouse, destination, value_))

    logger.info('\n')
    for ((period, product, factory, destination), var) in model.var_movement_factory.items():
        value_ = pyo.value(var)
        if value_ != 0:
            logger.info('var_movement_factory({},{},{},{}):{}'.format(period, product, factory, destination, value_))

    logger.info('\n')
    for ((period, product, warehouse), var) in model.stock_warehouse.items():
        value_ = pyo.value(var)
        if value_ != 0:
            logger.info('stock_warehouse({},{},{}):{}'.format(period, product, warehouse, value_))

    logger.info('\n')
    logger.info("Total cost:", pyo.value(model.objetive))

elif results.solver.termination_condition == TerminationCondition.infeasible:
    logger.info('(!) Infeasible solution')
else:
    logger.info('(!)')
    logger.info('Status:', results.solver.status)
    # Something else is wrong
    logger.info('termination condition:', results.solver.termination_condition)
