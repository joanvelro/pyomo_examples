import pyomo.environ as pyo


def create_model():
    model = pyo.AbstractModel()
    # Define the sets
    model.markets = pyo.Set()
    model.factories = pyo.Set()

    # Define the variables
    model.production = pyo.Param(model.factories)
    model.demand = pyo.Param(model.markets, mutable=True)  # mutable to change the values with instance its created

    # Define the matrix of cost of send one profuct from factories to markets
    model.costs = pyo.Param(model.factories, model.markets)

    # Define the variable of units of product from factory $i$ to market $j$ $x_{i,j}$
    model.units = pyo.Var(model.factories, model.markets, bounds=(0, 1000))

    # Define objective function (1)
    def Totalcost(model):
        return sum(model.costs[n, i] * model.units[n, i]
                   for n in model.factories
                   for i in model.markets)

    # Define objective
    model.finalcost = pyo.Objective(rule=Totalcost)

    # Define constraint function
    def MinDemand(model, markets):
        return sum(model.units[i, markets] for i in model.factories) >= model.demand[markets]

    # Define constraint function
    def MaxProduction(model, factories):
        return sum(model.units[factories, j] for j in model.markets) <= model.production[factories]

    # Define constraints
    model.DemandConstraint = pyo.Constraint(model.markets, rule=MinDemand)
    model.ProductionConstraint = pyo.Constraint(model.factories, rule=MaxProduction)

    return model


model = create_model()

instance = model.create_instance('transport_data_v1.dat')

instance.demand['Alicante'].value = 100
instance.demand['Castellon'] = 80

optimizer = pyo.SolverFactory('glpk')
# optimizer.options['max_iter'] = 100000

results = optimizer.solve(instance, tee=True)

print(results)
# results.write()

print('Results')
for f in instance.factories:
    for m in instance.markets:
        if instance.units[(f, m)] != 0:
            print('From factory', f, 'to market', m, 'x:', instance.units[(f, m)].value)

# instance.finalcost.expr()
print('total cost:', pyo.value(instance.finalcost), 'euros')
