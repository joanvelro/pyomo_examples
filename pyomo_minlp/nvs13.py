#  MINLP written by GAMS Convert at 05/24/17 13:23:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        5        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        1        0        5        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31        1       30        0
# 
#  Reformulation has removed 1 variable and 1 equation


from pyomo.environ import *

model = m = ConcreteModel()


m.i1 = Var(within=Integers,bounds=(0,200),initialize=1)
m.i2 = Var(within=Integers,bounds=(0,200),initialize=1)
m.i3 = Var(within=Integers,bounds=(0,200),initialize=1)
m.i4 = Var(within=Integers,bounds=(0,200),initialize=1)
m.i5 = Var(within=Integers,bounds=(0,200),initialize=1)

m.obj = Objective(expr=7*m.i1**2 + 6*m.i2**2 + 12*m.i1 - 77.2*m.i2 + 8*m.i3**2 - 6*m.i3*m.i1 + 4*m.i3*m.i2 - 19.2*m.i3
                        + 6*m.i4**2 + 2*m.i4*m.i1 + 2*m.i4*m.i3 - 36.6*m.i4 + 7*m.i5**2 - 4*m.i5*m.i1 - 2*m.i5*m.i2 - 6*
                       m.i5*m.i3 - 69.4*m.i5, sense=minimize)

m.c1 = Constraint(expr=(-9*m.i1**2) - 10*m.i1*m.i2 - 8*m.i2**2 - 5*m.i3**2 - 6*m.i3*m.i1 - 10*m.i3*m.i2 - 7*m.i4**2 - 10
                       *m.i4*m.i1 - 6*m.i4*m.i2 - 2*m.i4*m.i3 - 2*m.i5*m.i2 - 7*m.i5**2 >= -1430)

m.c2 = Constraint(expr=(-6*m.i1**2) - 8*m.i1*m.i2 - 6*m.i2**2 - 4*m.i3**2 - 2*m.i3*m.i1 - 2*m.i3*m.i2 - 8*m.i4**2 + 2*
                       m.i4*m.i1 + 10*m.i4*m.i2 - 2*m.i5*m.i1 - 6*m.i5*m.i2 + 6*m.i5*m.i4 + 7*m.i5**2 >= -1150)

m.c3 = Constraint(expr=(-9*m.i1**2) - 6*m.i2**2 - 8*m.i3**2 + 2*m.i2*m.i1 + 2*m.i3*m.i2 - 6*m.i4**2 + 4*m.i4*m.i1 + 4*
                       m.i4*m.i2 - 2*m.i4*m.i3 - 6*m.i5*m.i1 - 2*m.i5*m.i2 + 4*m.i5*m.i4 + 6*m.i5**2 >= -850)

m.c4 = Constraint(expr=(-8*m.i1**2) - 4*m.i2**2 - 9*m.i3**2 - 7*m.i4**2 - 2*m.i2*m.i1 - 2*m.i3*m.i1 - 4*m.i3*m.i2 + 6*
                       m.i4*m.i1 + 2*m.i4*m.i2 - 2*m.i4*m.i3 - 6*m.i5*m.i1 - 4*m.i5*m.i2 - 2*m.i5*m.i3 + 6*m.i5*m.i4 + 6
                       *m.i5**2 >= -1125)

m.c5 = Constraint(expr=2*m.i2*m.i1 - 4*m.i1**2 - 5*m.i2**2 - 6*m.i3*m.i1 - 8*m.i3**2 - 2*m.i4*m.i1 + 6*m.i4*m.i2 - 2*
                       m.i4*m.i3 - 6*m.i4**2 - 4*m.i5*m.i1 + 2*m.i5*m.i2 - 6*m.i5*m.i3 - 8*m.i5*m.i4 - 7*m.i5**2
                        >= -1030)
