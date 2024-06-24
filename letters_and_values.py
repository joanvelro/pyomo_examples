from pyomo.environ import *

m = ConcreteModel()

m.letters = Set(initialize=['S', 'E', 'N', 'D', 'M', 'O', 'R', 'Y'])
m.vals = RangeSet(0, 9)
m.tens = Set(initialize=['C1', 'C2', 'C3', 'C4'])

m.ten_val = Var(m.tens, within=Binary, initialize=0)
m.x = Var(m.letters, m.vals, within=Binary, initialize=1)


def c_rule(m, i):
    return sum(m.x[i, j] for j in m.vals) <= 1


m.col = Constraint(m.letters, rule=c_rule)  # not more than one value for the letter


def r_rule(m, j):
    return sum(m.x[i, j] for i in m.letters) <= 1


m.row = Constraint(m.vals, rule=r_rule)  # not more than one value for the digit

m.C1 = Constraint(expr=m.ten_val['C4'] == m.x['M', 1])
m.C2 = Constraint(expr=m.x['M', 1] == 1)
m.C3 = Constraint(
    expr=m.ten_val['C3'] + sum(m.x['S', j] * j for j in m.vals) + m.x['M', 1] == sum(m.x['O', j] * j for j in m.vals) +
         m.ten_val['C4'] * 10)
m.C4 = Constraint(
    expr=m.ten_val['C2'] + sum(m.x['E', j] * j for j in m.vals) + sum(m.x['O', j] * j for j in m.vals) == sum(
        m.x['N', j] * j for j in m.vals) + m.ten_val['C3'] * 10)
m.C5 = Constraint(
    expr=m.ten_val['C1'] + sum(m.x['N', j] * j for j in m.vals) + sum(m.x['R', j] * j for j in m.vals) == sum(
        m.x['E', j] * j for j in m.vals) + m.ten_val['C2'] * 10)
m.C6 = Constraint(expr=sum(m.x['D', j] * j for j in m.vals) + sum(m.x['E', j] * j for j in m.vals) == sum(
    m.x['Y', j] * j for j in m.vals) + m.ten_val['C1'] * 10)

m.f1 = Objective(expr=sum(m.x['S', j] * j for j in m.vals),
                 sense=minimize)  # we are searching for working solution, no matter what to minimize

solver = SolverFactory('glpk')
results = solver.solve(m, tee=True)


print(results.Problem())
print(results.Solver())
m.x.pprint()
m.ten_val.pprint()
print('hi')