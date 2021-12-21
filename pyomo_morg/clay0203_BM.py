#  MINLP written by GAMS Convert at 05/24/17 13:23:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         49        1       12       36        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         13       13        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        115       67       48        0
# 
#  Reformulation has removed 1 variable and 1 equation


from pyomo.environ import *

model = m = ConcreteModel()


m.x2 = Var(within=Reals,bounds=(11.5,52.5),initialize=11.5)
m.x3 = Var(within=Reals,bounds=(12.5,51.5),initialize=12.5)
m.x4 = Var(within=Reals,bounds=(10.5,53.5),initialize=10.5)
m.x5 = Var(within=Reals,bounds=(7,82),initialize=7)
m.x6 = Var(within=Reals,bounds=(6.5,82.5),initialize=6.5)
m.x7 = Var(within=Reals,bounds=(5.5,83.5),initialize=5.5)
m.x8 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x9 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x10 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x11 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x12 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x13 = Var(within=Reals,bounds=(0,None),initialize=0)

m.obj = Objective(expr=   300*m.x8 + 240*m.x9 + 100*m.x10 + 300*m.x11 + 240*m.x12 + 100*m.x13, sense=minimize)

m.c2 = Constraint(expr= - m.x2 + m.x3 + m.x8 >= 0)

m.c3 = Constraint(expr=   m.x2 - m.x3 + m.x8 >= 0)

m.c4 = Constraint(expr= - m.x5 + m.x6 + m.x11 >= 0)

m.c5 = Constraint(expr=   m.x5 - m.x6 + m.x11 >= 0)

m.c6 = Constraint(expr= - m.x3 + m.x4 + m.x10 >= 0)

m.c7 = Constraint(expr=   m.x3 - m.x4 + m.x10 >= 0)

m.c8 = Constraint(expr= - m.x6 + m.x7 + m.x13 >= 0)

m.c9 = Constraint(expr=   m.x6 - m.x7 + m.x13 >= 0)

m.c10 = Constraint(expr= - m.x2 + m.x4 + m.x9 >= 0)

m.c11 = Constraint(expr=   m.x2 - m.x4 + m.x9 >= 0)

m.c12 = Constraint(expr= - m.x5 + m.x7 + m.x12 >= 0)

m.c13 = Constraint(expr=   m.x5 - m.x7 + m.x12 >= 0)

m.c14 = Constraint(expr=   m.x2 - m.x3 <= -6)

m.c15 = Constraint(expr= - m.x2 + m.x3 <= -6)

m.c16 = Constraint(expr=   m.x5 - m.x6 <= -5.5)

m.c17 = Constraint(expr= - m.x5 + m.x6 <= -5.5)

m.c18 = Constraint(expr=   m.x3 - m.x4 <= -5)

m.c19 = Constraint(expr= - m.x3 + m.x4 <= -5)

m.c20 = Constraint(expr=   m.x6 - m.x7 <= -4)

m.c21 = Constraint(expr= - m.x6 + m.x7 <= -4)

m.c22 = Constraint(expr=   m.x2 - m.x4 <= -4)

m.c23 = Constraint(expr= - m.x2 + m.x4 <= -4)

m.c24 = Constraint(expr=   m.x5 - m.x7 <= -4.5)

m.c25 = Constraint(expr= - m.x5 + m.x7 <= -4.5)

m.c26 = Constraint(expr=(-17.5 + m.x2)**2 + (-7 + m.x5)**2 <= 36)

m.c27 = Constraint(expr=(-17.5 + m.x2)**2 + (-13 + m.x5)**2 <= 36)

m.c28 = Constraint(expr=(-12.5 + m.x2)**2 + (-7 + m.x5)**2 <= 36)

m.c29 = Constraint(expr=(-12.5 + m.x2)**2 + (-13 + m.x5)**2 <= 36)

m.c30 = Constraint(expr=(-52.5 + m.x2)**2 + (-77 + m.x5)**2 <= 25)

m.c31 = Constraint(expr=(-52.5 + m.x2)**2 + (-83 + m.x5)**2 <= 25)

m.c32 = Constraint(expr=(-47.5 + m.x2)**2 + (-77 + m.x5)**2 <= 25)

m.c33 = Constraint(expr=(-47.5 + m.x2)**2 + (-83 + m.x5)**2 <= 25)

m.c34 = Constraint(expr=(-18.5 + m.x3)**2 + (-7.5 + m.x6)**2 <= 36)

m.c35 = Constraint(expr=(-18.5 + m.x3)**2 + (-12.5 + m.x6)**2 <= 36)

m.c36 = Constraint(expr=(-11.5 + m.x3)**2 + (-7.5 + m.x6)**2 <= 36)

m.c37 = Constraint(expr=(-11.5 + m.x3)**2 + (-12.5 + m.x6)**2 <= 36)

m.c38 = Constraint(expr=(-53.5 + m.x3)**2 + (-77.5 + m.x6)**2 <= 25)

m.c39 = Constraint(expr=(-53.5 + m.x3)**2 + (-82.5 + m.x6)**2 <= 25)

m.c40 = Constraint(expr=(-46.5 + m.x3)**2 + (-77.5 + m.x6)**2 <= 25)

m.c41 = Constraint(expr=(-46.5 + m.x3)**2 + (-82.5 + m.x6)**2 <= 25)

m.c42 = Constraint(expr=(-16.5 + m.x4)**2 + (-8.5 + m.x7)**2 <= 36)

m.c43 = Constraint(expr=(-16.5 + m.x4)**2 + (-11.5 + m.x7)**2 <= 36)

m.c44 = Constraint(expr=(-13.5 + m.x4)**2 + (-8.5 + m.x7)**2 <= 36)

m.c45 = Constraint(expr=(-13.5 + m.x4)**2 + (-11.5 + m.x7)**2 <= 36)

m.c46 = Constraint(expr=(-51.5 + m.x4)**2 + (-78.5 + m.x7)**2 <= 25)

m.c47 = Constraint(expr=(-51.5 + m.x4)**2 + (-81.5 + m.x7)**2 <= 25)

m.c48 = Constraint(expr=(-48.5 + m.x4)**2 + (-78.5 + m.x7)**2 <= 25)

m.c49 = Constraint(expr=(-48.5 + m.x4)**2 + (-81.5 + m.x7)**2 <= 25)
