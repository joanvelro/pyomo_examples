#  MINLP written by GAMS Convert at 05/24/17 13:23:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         56       51        2        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         50       40       10        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        130      123        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


from pyomo.environ import *

model = m = ConcreteModel()


m.b2 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b3 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b4 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b5 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b6 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b7 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b8 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b9 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b10 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b11 = Var(within=Binary,bounds=(0,1),initialize=0)
m.x12 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x13 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x14 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x15 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x16 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x17 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x18 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x19 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x20 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x21 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x22 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x23 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x24 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x25 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x26 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x27 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x28 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x29 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x30 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x31 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x32 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x33 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x34 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x35 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x36 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x37 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x38 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x39 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x40 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x41 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x42 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x43 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x44 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x45 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x46 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x47 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x48 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x49 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x50 = Var(within=Reals,bounds=(0,None),initialize=0)

m.obj = Objective(expr=   m.x13 - 10*m.x14 + m.x15 - 15*m.x16 - 40*m.x20 + 15*m.x21 + 15*m.x25 + 80*m.x28 - 65*m.x29
                        + 25*m.x30 - 60*m.x31 + 35*m.x32 - 80*m.x33 - 35*m.x36 - 70*m.x38 - 65*m.x40 + m.x41 + m.x42
                        + m.x43 + m.x44 + m.x45 + m.x46 + m.x47 + m.x48 + m.x49 + m.x50 + 80, sense=minimize)

m.c1 = Constraint(expr=   m.x24 - m.x30 - m.x32 - m.x37 == 0)

m.c2 = Constraint(expr= - m.x20 - m.x27 + m.x28 - m.x36 == 0)

m.c3 = Constraint(expr=   m.x22 - m.x23 - m.x26 - m.x39 == 0)

m.c4 = Constraint(expr=   m.x14 + m.x16 - m.x17 - m.x22 == 0)

m.c5 = Constraint(expr=   m.x17 - m.x18 - m.x19 == 0)

m.c6 = Constraint(expr= - m.x31 - m.x33 + m.x34 - m.x38 - m.x40 == 0)

m.c7 = Constraint(expr= - m.x25 + m.x34 - m.x35 == 0)

m.c8 = Constraint(expr=   m.x12 - m.x13 - m.x15 == 0)

m.c9 = Constraint(expr=   m.x21 - 0.8*m.x28 <= 0)

m.c10 = Constraint(expr=   m.x21 - 0.4*m.x28 >= 0)

m.c11 = Constraint(expr=   m.x23 - 5*m.x25 <= 0)

m.c12 = Constraint(expr=   m.x23 - 2*m.x25 >= 0)

m.c13 = Constraint(expr=exp(m.x14) - m.x13 == 1)

m.c14 = Constraint(expr=   m.x13 == 0)

m.c15 = Constraint(expr=   m.x14 == 0)

m.c16 = Constraint(expr=   m.x41 == 32)

m.c17 = Constraint(expr=exp(0.833333333333333*m.x16) - m.x15 == 1)

m.c18 = Constraint(expr=   m.x15 == 0)

m.c19 = Constraint(expr=   m.x16 == 0)

m.c20 = Constraint(expr=   m.x42 == 18)

m.c21 = Constraint(expr= - m.x19 + 1.5*m.x20 + m.x21 == 0)

m.c22 = Constraint(expr=   m.x20 == 0)

m.c23 = Constraint(expr=   m.x43 == 12)

m.c24 = Constraint(expr=   1.25*m.x23 - m.x24 + 1.25*m.x25 == 0)

m.c25 = Constraint(expr=   m.x23 == 0)

m.c26 = Constraint(expr=   m.x24 == 0)

m.c27 = Constraint(expr=   m.x25 == 0)

m.c28 = Constraint(expr=   m.x44 == 14)

m.c29 = Constraint(expr=   m.x26 - 2*m.x27 == 0)

m.c30 = Constraint(expr=   m.x26 == 0)

m.c31 = Constraint(expr=   m.x27 == 0)

m.c32 = Constraint(expr=   m.x45 == 2)

m.c33 = Constraint(expr=exp(0.666666666666667*m.x31) - m.x30 == 1)

m.c34 = Constraint(expr=   m.x30 == 0)

m.c35 = Constraint(expr=   m.x31 == 0)

m.c36 = Constraint(expr=   m.x46 == 41)

m.c37 = Constraint(expr=exp(m.x33) - m.x32 == 1)

m.c38 = Constraint(expr=   m.x32 == 0)

m.c39 = Constraint(expr=   m.x33 == 0)

m.c40 = Constraint(expr=   m.x47 == 32)

m.c41 = Constraint(expr=exp(m.x29) - m.x21 - m.x28 == 1)

m.c42 = Constraint(expr=   m.x21 == 0)

m.c43 = Constraint(expr=   m.x28 == 0)

m.c44 = Constraint(expr=   m.x29 == 0)

m.c45 = Constraint(expr=   m.x36 == 0)

m.c46 = Constraint(expr=   m.x48 == 25)

m.c47 = Constraint(expr=exp(0.833333333333333*m.x38) - m.x37 == 1)

m.c48 = Constraint(expr=   m.x37 == 0)

m.c49 = Constraint(expr=   m.x38 == 0)

m.c50 = Constraint(expr=   m.x49 == 6)

m.c51 = Constraint(expr=exp(0.526315789473684*m.x40) - m.x39 == 1)

m.c52 = Constraint(expr=   m.x40 == 0)

m.c53 = Constraint(expr=   m.x39 == 0)

m.c54 = Constraint(expr=   m.x50 == 1)

m.c55 = Constraint(expr=   m.b2 + m.b3 + m.b4 + m.b5 + m.b6 + m.b7 + m.b8 + m.b9 + m.b10 + m.b11 <= 12)
