""" Solve a transportation problem using pyomo """
import pandas as pd
import pyomo.environ as pyo


class engine(object):

    def __init__(self, costs, locations):
        self.model = None
        self.costs = costs
        self.locations = locations

    def build_model(self):
        """
        Build model
        """
        self.model = pyo.ConcreteModel()
        # 4. Define the decision variables
        self.model.x = pyo.Var(self.locations.index, self.locations.index, within=pyo.NonNegativeReals)
        # 5. Define the objective function
        self.model.cost = pyo.Objective(expr=sum(self.model.x[i, j] * self.costs.loc[i, j]
                                                 for i in self.locations.index
                                                 for j in self.locations.index))
        # 6. Define the constraints
        self.model.supply = pyo.Constraint(expr=sum(self.model.x[i, 'Chicago']
                                                    for i in self.locations.index) == 10)
        self.model.demand = pyo.Constraint(expr=sum(self.model.x['New York', j]
                                                    for j in self.locations.index) == 10)
        return self.model


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
    model = engine(costs, locations)
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


if __name__ == "__main__":
    solve_problem()
