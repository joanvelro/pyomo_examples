import pyomo.environ as pyo
from numpy import sqrt
from pyomo.opt import SolverFactory


class Engine(object):

    def __init__(self, costs: dict, stock: dict, demand: dict, products: list):
        self.results = None
        self.model = None
        self.costs = costs
        self.stock = stock
        self.demand = demand
        self.products = products

    def execute(self, solver: str = 'glpk', verbose: bool = True):

        self.build_model()
        self.solve_problem(solver=solver, verbose=verbose)
        return self.results

    def build_model(self):
        """
        Build model
        """
        self.model = pyo.ConcreteModel()
        self.model.I = pyo.Set(initialize=self.products, dimen=1)
        self.model.x = pyo.Var(self.model.I, domain=pyo.PositiveIntegers)
        self.model.slack = pyo.Var(self.model.I, domain=pyo.PositiveIntegers)
        self.model.b = pyo.Var(self.model.I, domain=pyo.Binary)

        self.model.OBJ = pyo.Objective(expr=sum(self.costs[i] * self.model.x[i] ** 2
                                                for i in self.model.I),
                                       sense=pyo.minimize)

        # self.model.Constraint1 = \
        #    pyo.Constraint(expr=sum(self.a[i] * self.model.x[i] + self.model.slack[i] for i in self.model.I) >= self.b)

        self.model.constraint_supply = pyo.ConstraintList()
        for i in self.model.I:
            self.model.constraint_supply.add(self.model.x[i]**(1/2) >= self.demand[i])

        self.model.constraint_LL = pyo.Constraint(self.model.I, rule=self.__constraint_LL)

    def __constraint_LL(self, model, i):
        return self.model.x[i] <= self.model.b[i] * self.stock[i]

    def print_model(self):
        print(self.model.pprint())

    def solve_problem(self, solver: str = 'glpk', verbose: bool = True):
        if solver == 'scip':
            opt = SolverFactory(solver, solver_io='nl')
        else:
            opt = SolverFactory(solver)
        self.results = opt.solve(self.model, tee=verbose)

    def print_solution(self):

        print('\n')
        print('Objective:', pyo.value(self.model.OBJ))
        print('\n')
        for i in self.model.I:
            print('x[{}]: {}'.format(i, self.model.x[i].value))

        print('\n')
        for i in self.model.I:
            print('b[{}]: {}'.format(i, self.model.b[i].value))

        print('\n')
        for i in self.model.I:
            print('Slack[{}]: {}'.format(i, self.model.slack[i].value))


if __name__ == "__main__":
    # define input data
    products = ['A', 'B', 'C', 'D', 'E']
    costs = {'A': 30, 'B': 40, 'C': 50, 'D': 60, 'E': 70}  # costs of sending each product
    stock = {'A': 543, 'B': 1000, 'C': 1000, 'D': 1000, 'E': 1000}  # stock products
    demand = {'A': 456, 'B': 560, 'C': 321, 'D': 567, 'E': 234}  # demand of each product

    # Build and solve model
    engine = Engine(costs=costs, demand=demand, stock=stock, products=products)
    engine.build_model()
    # engine.print_model()
    engine.solve_problem(solver='scip')
    engine.print_solution()

    # results = engine.execute(solver='ipopt')

    # send results to stdout
    # results.write()
