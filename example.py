""" Solve a transportation problem using pyomo """
import pandas as pd


def solve_problem():
    import pyomo.environ as pyo
    """
    1. Read problem location files
    2. Read problem costs
    """  # Hasta aqui he especificado yo, lo demás lo ha copiado evidentemente !!

    # 1. Read problem location files
    locations = pd.read_csv('locations.csv')
    locations.set_index('Location', inplace=True)
    # 2. Read problem costs
    costs = pd.read_csv('costs.csv')
    costs.set_index(['From', 'To'], inplace=True)
    # 3. Create a pyomo model
    model = pyo.ConcreteModel()
    # 4. Define the decision variables
    model.x = pyo.Var(locations.index, locations.index,
                      within=pyo.NonNegativeReals)
    # 5. Define the objective function
    model.cost = pyo.Objective(expr=sum(model.x[i, j] * costs.loc[i, j]
                                        for i in locations.index
                                        for j in locations.index))
    # 6. Define the constraints
    model.supply = pyo.Constraint(expr=sum(model.x[i, 'Chicago']
                                           for i in locations.index) == 10)
    model.demand = pyo.Constraint(expr=sum(model.x['New York', j]
                                           for j in locations.index) == 10)
    # 7. Solve the problem
    solver = pyo.SolverFactory('glpk')
    solver.solve(model)
    # 8. Extract the solution
    solution = pd.DataFrame.from_dict(model.x, orient='index')
    solution.index.name = 'Location'
    solution.columns = ['Shipment']
    solution.reset_index(inplace=True)
    solution['Shipment'] = solution['Shipment'].map(lambda x: x.value)
    solution.to_csv('solution.csv', index=False)

    return solution
