#  MINLP written by GAMS Convert at 05/24/17 13:23:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         64       59        2        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         57       45       12        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        148      139        9        0
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
m.b12 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b13 = Var(within=Binary,bounds=(0,1),initialize=0)
m.x14 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x15 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x16 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x17 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x18 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x19 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x20 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x21 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x22 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x23 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x24 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x25 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x26 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x27 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x28 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x29 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x30 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x31 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x32 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x33 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x34 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x35 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x36 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x37 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x38 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x39 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x40 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x41 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x42 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x43 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x44 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x45 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x46 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x47 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x48 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x49 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x50 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x51 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x52 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x53 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x54 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x55 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x56 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x57 = Var(within=Reals,bounds=(0,None),initialize=0)

m.obj = Objective(expr=   m.x15 - 10*m.x16 + m.x17 - 15*m.x18 - 40*m.x22 + 15*m.x23 + 15*m.x27 + 80*m.x30 - 65*m.x31
                        + 25*m.x32 - 60*m.x33 + 35*m.x34 - 80*m.x35 - 35*m.x38 - 70*m.x40 - 65*m.x42 + 1.2*m.x44
                        - 2*m.x45 + m.x46 + m.x47 + m.x48 + m.x49 + m.x50 + m.x51 + m.x52 + m.x53 + m.x54 + m.x55
                        + m.x56 + m.x57 + 122, sense=minimize)

m.c1 = Constraint(expr=   m.x26 - m.x32 - m.x34 - m.x39 == 0)

m.c2 = Constraint(expr= - m.x22 - m.x29 + m.x30 - m.x38 == 0)

m.c3 = Constraint(expr=   m.x24 - m.x25 - m.x28 - m.x41 == 0)

m.c4 = Constraint(expr=   m.x16 + m.x18 - m.x19 - m.x24 + m.x44 == 0)

m.c5 = Constraint(expr=   m.x19 - m.x20 - m.x21 == 0)

m.c6 = Constraint(expr= - m.x33 - m.x35 + m.x36 - m.x40 - m.x42 == 0)

m.c7 = Constraint(expr= - m.x27 + m.x36 - m.x37 == 0)

m.c8 = Constraint(expr=   m.x14 - m.x15 - m.x17 - m.x43 == 0)

m.c9 = Constraint(expr=   m.x23 - 0.8*m.x30 <= 0)

m.c10 = Constraint(expr=   m.x23 - 0.4*m.x30 >= 0)

m.c11 = Constraint(expr=   m.x25 - 5*m.x27 <= 0)

m.c12 = Constraint(expr=   m.x25 - 2*m.x27 >= 0)

m.c13 = Constraint(expr=exp(m.x16) - m.x15 == 1)

m.c14 = Constraint(expr=   m.x15 == 0)

m.c15 = Constraint(expr=   m.x16 == 0)

m.c16 = Constraint(expr=   m.x46 == 5)

m.c17 = Constraint(expr=exp(0.833333333333333*m.x18) - m.x17 == 1)

m.c18 = Constraint(expr=   m.x17 == 0)

m.c19 = Constraint(expr=   m.x18 == 0)

m.c20 = Constraint(expr=   m.x47 == 8)

m.c21 = Constraint(expr= - m.x21 + 1.5*m.x22 + m.x23 == 0)

m.c22 = Constraint(expr=   m.x22 == 0)

m.c23 = Constraint(expr=   m.x48 == 6)

m.c24 = Constraint(expr=   1.25*m.x25 - m.x26 + 1.25*m.x27 == 0)

m.c25 = Constraint(expr=   m.x25 == 0)

m.c26 = Constraint(expr=   m.x26 == 0)

m.c27 = Constraint(expr=   m.x27 == 0)

m.c28 = Constraint(expr=   m.x49 == 10)

m.c29 = Constraint(expr=   m.x28 - 2*m.x29 == 0)

m.c30 = Constraint(expr=   m.x28 == 0)

m.c31 = Constraint(expr=   m.x29 == 0)

m.c32 = Constraint(expr=   m.x50 == 6)

m.c33 = Constraint(expr=exp(0.666666666666667*m.x33) - m.x32 == 1)

m.c34 = Constraint(expr=   m.x32 == 0)

m.c35 = Constraint(expr=   m.x33 == 0)

m.c36 = Constraint(expr=   m.x51 == 7)

m.c37 = Constraint(expr=exp(m.x35) - m.x34 == 1)

m.c38 = Constraint(expr=   m.x34 == 0)

m.c39 = Constraint(expr=   m.x35 == 0)

m.c40 = Constraint(expr=   m.x52 == 4)

m.c41 = Constraint(expr=exp(m.x31) - m.x23 - m.x30 == 1)

m.c42 = Constraint(expr=   m.x23 == 0)

m.c43 = Constraint(expr=   m.x30 == 0)

m.c44 = Constraint(expr=   m.x31 == 0)

m.c45 = Constraint(expr=   m.x38 == 0)

m.c46 = Constraint(expr=   m.x53 == 5)

m.c47 = Constraint(expr=exp(0.833333333333333*m.x40) - m.x39 == 1)

m.c48 = Constraint(expr=   m.x39 == 0)

m.c49 = Constraint(expr=   m.x40 == 0)

m.c50 = Constraint(expr=   m.x54 == 6)

m.c51 = Constraint(expr=exp(m.x45) - m.x20 == 1)

m.c52 = Constraint(expr=   m.x45 == 0)

m.c53 = Constraint(expr=   m.x20 == 0)

m.c54 = Constraint(expr=   m.x55 == 3)

m.c55 = Constraint(expr=exp(0.526315789473684*m.x42) - m.x41 == 1)

m.c56 = Constraint(expr=   m.x42 == 0)

m.c57 = Constraint(expr=   m.x41 == 0)

m.c58 = Constraint(expr=   m.x56 == 1)

m.c59 = Constraint(expr=exp(0.909090909090909*m.x44) - m.x43 == 1)

m.c60 = Constraint(expr=   m.x44 == 0)

m.c61 = Constraint(expr=   m.x43 == 0)

m.c62 = Constraint(expr=   m.x57 == 7)

m.c63 = Constraint(expr=   m.b2 + m.b3 + m.b4 + m.b5 + m.b6 + m.b7 + m.b8 + m.b9 + m.b10 + m.b11 + m.b12 + m.b13 <= 12)
