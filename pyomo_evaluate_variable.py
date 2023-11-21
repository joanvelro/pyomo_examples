from pyomo.environ import *


# Definir función externa
def evaluate_variable_value(variable):
    value = variable.value
    print("Value of variable:", value)


# Crear modelo
model = ConcreteModel()

# Crear variable de decisión
model.x = Var(domain=NonNegativeReals)

# Restricción
model.constraint = Constraint(expr=model.x >= 10)

# Solucionar el modelo
solver = SolverFactory('glpk')
results = solver.solve(model)

# Evaluar el valor de la variable de decisión durante la ejecución
current_value = model.x.value
print("Current value of x:", current_value)

# Modificar el valor de la variable de decisión
model.x.value = 15

# Llamar a la función externa para evaluar el valor de la variable de decisión
evaluate_variable_value(model.x)
