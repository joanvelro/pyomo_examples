"""
Single Stage Transport Problem
"""
import pyomo.environ as pyo
import json
from pandas import DataFrame


def build_input_instance(path):
    """
    TBD
    """
    with open(path, 'r') as file:
        request_data = json.load(file)

        request_data['Parameters']['Costs'] = {(j, i): request_data['Parameters']['Costs'][j][i]
                                               for j in request_data['Sets']['LogisticCentres']
                                               for i in request_data['Sets']['Factories']}
    return request_data


def build_model(input_data):
    """
    TBD
    """
    model = pyo.ConcreteModel()

    # sets
    model.I = pyo.Set(dimen=1, initialize=input_data['Sets']['Factories'])  # Factories
    model.J = pyo.Set(dimen=1, initialize=input_data['Sets']['LogisticCentres'])  # Logistic Centres

    # parameters
    model.D = pyo.Param(model.I, initialize=input_data['Parameters']['Demand'])
    model.P = pyo.Param(model.J, initialize=input_data['Parameters']['Production'])
    model.A = pyo.Param(model.J, model.I, initialize=input_data['Parameters']['Costs'])

    # variables
    model.x = pyo.Var(model.J, model.I, domain=pyo.NonNegativeReals)

    # minimize transportation costs (objective)
    def objfunc(model):
        return pyo.summation(model.A, model.x)

    model.objective = pyo.Objective(rule=objfunc, sense=pyo.minimize)

    # Supply demand constraint
    def const1(model, i):
        return sum(model.x[j, i] for j in model.J) >= model.D[i]

    model.CONST1 = pyo.Constraint(model.I, rule=const1)

    # Limit Production constraint
    def const2(model, j):
        return sum(model.x[j, i] for i in model.I) <= model.P[j]

    model.CONST2 = pyo.Constraint(model.J, rule=const2)

    return model


def generate_solver_factory(solver='cbc'):
    """
    TBD
    """
    # Define solver: glpk, cbc, cplex, gurobi

    opt = pyo.SolverFactory(solver)

    if solver == 'glpk':
        opt.options['mipgap'] = 0.001

    elif solver == 'cbc':
        opt.options['integertolerance'] = 1e-11
        opt.options['ratioGAP'] = 0.0005  # (0.05 %)
        opt.options['timeMode'] = 'elapsed'
        opt.options['seconds'] = 300

    elif solver == 'cplex':
        opt.options['mipgap'] = 0.001

    return opt


def build_movements_solution():
    X = {key: pyo.value(value) for key, value in model.x.items()}

    solution_data = []
    for i in input_data['Sets']['Factories']:
        solution_data.append([X[(j, i)] for j in input_data['Sets']['LogisticCentres']])

    df_solution = DataFrame(data=solution_data,
                            columns=input_data['Sets']['LogisticCentres'],
                            index=input_data['Sets']['Factories'])

    return df_solution


def has_solution(results):

    status = results.solver.status
    condition = results.solver.termination_condition
    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    return solution


if __name__ == "__main__":
    input_data = build_input_instance('single_stage_transport_problem.json')
    model = build_model(input_data)
    opt = generate_solver_factory(solver='glpk')
    verbosity = False
    results = opt.solve(model, tee=verbosity)
    solution = has_solution(results)
    if solution:
        print('Optimal Transportation cost: {} M€'.format(pyo.value(model.objective)/1000000))
        df_moves_solution = build_movements_solution()
