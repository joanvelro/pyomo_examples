from pyomo.environ import *

model = AbstractModel()

model.x = Var(initialize=1.0)

def _e(m,i):
  return m.x*i
model.e = Expression([1,2,3], rule=_e)

instance = model.create_instance()

print(value(instance.e[1])) # -> 1.0
print(instance.e[1]())      # -> 1.0
print(instance.e[1].expr)   # -> a pyomo expression object

# Change the underlying expression
instance.e[1].set_value(instance.x**2)

#... solve
#... load results

# print the value of the expression given the loaded optimal solution
print(value(instance.e[1]))
