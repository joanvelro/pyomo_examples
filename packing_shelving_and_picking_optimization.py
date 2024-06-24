from pyomo.environ import *

from utils import has_solution

# Create a Concrete Model
model = ConcreteModel()

# Sets
Products = ['Product1',
            'Product2',
            'Product3']  # Example products
Orders = ['Order1',
          'Order2',
          'Order3']  # Example orders
Shelves = ['Shelf1',
           'Shelf2',
           'Shelf3']  # Example shelves

# Parameters
packing_time = {'Product1': 1,
                'Product2': 2,
                'Product3': 3}  # Time to pack each product
picking_time = {'Order1': {'Product1': 2,
                           'Product2': 1,
                           'Product3': 3},
                'Order2': {'Product1': 3,
                           'Product2': 2,
                           'Product3': 1},
                'Order3': {'Product1': 1,
                           'Product2': 3,
                           'Product3': 2}}  # Time to pick each product for each order
shelf_capacity = {'Shelf1': 100,
                  'Shelf2': 150,
                  'Shelf3': 200}  # Maximum weight capacity of each shelf

# Variables
model.Pack = Var(Products, within=Binary)  # 1 if product is packed, 0 otherwise
model.Pick = Var(Orders, Products, within=Binary)  # 1 if product is picked for order, 0 otherwise
model.Shelf = Var(Products, Shelves, within=Binary)  # 1 if product is shelved on shelf, 0 otherwise

# Objective Function: Minimize total packing and picking time
model.obj = Objective(expr=sum(packing_time[p] * model.Pack[p] for p in Products) +
                           sum(picking_time[o][p] * model.Pick[o, p] for o in Orders for p in Products),
                      sense=minimize)


# Constraints
# Packing constraints: Each product is packed exactly once
def pack_constraint_rule(model, p):
    return sum(model.Pick[o, p] for o in Orders) == model.Pack[p]


model.PackConstraint = Constraint(Products, rule=pack_constraint_rule)


# Shelf capacity constraint: Total weight on each shelf must not exceed its capacity
def shelf_capacity_constraint_rule(model, s):
    return sum(model.Shelf[p, s] for p in Products) <= shelf_capacity[s]


model.ShelfCapacityConstraint = Constraint(Shelves, rule=shelf_capacity_constraint_rule)


# Picking constraints: Each product must be picked if it is packed for any order
def picking_constraint_rule(model, o, p):
    return model.Pick[o, p] <= model.Pack[p]


model.PickingConstraint = Constraint(Orders, Products, rule=picking_constraint_rule)


# Shelf assignment constraint: If a product is packed, it must be shelved on exactly one shelf
def shelf_assignment_constraint_rule(model, p):
    return sum(model.Shelf[p, s] for s in Shelves) == model.Pack[p]


model.ShelfAssignmentConstraint = Constraint(Products, rule=shelf_assignment_constraint_rule)

# Solve the model
solver = SolverFactory('glpk')
results = solver.solve(model, tee=True)

if has_solution(results, model):

    print("Optimal packing, shelving, and picking plan found:")

    for (p, var) in model.Pack.items():
        print('{}:{}'.format(p, value(var)))

    print('\n')
    for ((o, p), var) in model.Pick.items():
        print('({},{}):{}'.format(o, p, value(var)))

    print('\n')

    for ((p, s), var) in model.Shelf.items():
        print('({},{}):{}'.format(p, s, value(var)))

    print('\n')
    for p in Products:
        if model.Pack[p].value > 0.5:
            print(f"Product {p} is packed.")
            for o in Orders:
                if model.Pick[o, p].value > 0.5:
                    print(f"- Picked for Order {o}")
            for s in Shelves:
                if model.Shelf[p, s].value > 0.5:
                    print(f"- Shelved on Shelf {s}")

    # Optional: Print total packing and picking time
    total_time = sum(packing_time[p] * model.Pack[p].value for p in Products) + \
                 sum(picking_time[o][p] * model.Pick[o, p].value for o in Orders for p in Products)
    print(f"Total time: {total_time}")
