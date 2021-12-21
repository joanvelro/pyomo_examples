#  MINLP written by GAMS Convert at 05/24/17 13:24:11
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         55       43        2       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         42       34        8        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        122      117        5        0
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
m.x10 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x11 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x12 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x13 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x14 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x15 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x16 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x17 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x18 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x19 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x20 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x21 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x22 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x23 = Var(within=Reals,bounds=(0,1),initialize=0)
m.x24 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x25 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x26 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x27 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x28 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x29 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x30 = Var(within=Reals,bounds=(0,2),initialize=0)
m.x31 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x32 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x33 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x34 = Var(within=Reals,bounds=(0,3),initialize=0)
m.x35 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x36 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x37 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x38 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x39 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x40 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x41 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x42 = Var(within=Reals,bounds=(0,None),initialize=0)

m.obj = Objective(expr=   m.x11 - 10*m.x12 + m.x13 - 15*m.x14 - 40*m.x18 + 15*m.x19 + 15*m.x23 + 80*m.x26 - 65*m.x27
                        + 25*m.x28 - 60*m.x29 + 35*m.x30 - 80*m.x31 - 35*m.x34 + m.x35 + m.x36 + m.x37 + m.x38 + m.x39
                        + m.x40 + m.x41 + m.x42 + 122, sense=minimize)

m.c1 = Constraint(expr=   m.x22 - m.x28 - m.x30 == 0)

m.c2 = Constraint(expr= - m.x18 - m.x25 + m.x26 - m.x34 == 0)

m.c3 = Constraint(expr=   m.x20 - m.x21 - m.x24 == 0)

m.c4 = Constraint(expr=   m.x12 + m.x14 - m.x15 - m.x20 == 0)

m.c5 = Constraint(expr=   m.x15 - m.x16 - m.x17 == 0)

m.c6 = Constraint(expr= - m.x29 - m.x31 + m.x32 == 0)

m.c7 = Constraint(expr= - m.x23 + m.x32 - m.x33 == 0)

m.c8 = Constraint(expr=   m.x10 - m.x11 - m.x13 == 0)

m.c9 = Constraint(expr=   m.x19 - 0.8*m.x26 <= 0)

m.c10 = Constraint(expr=   m.x19 - 0.4*m.x26 >= 0)

m.c11 = Constraint(expr=   m.x21 - 5*m.x23 <= 0)

m.c12 = Constraint(expr=   m.x21 - 2*m.x23 >= 0)

m.c13 = Constraint(expr= - 10*m.b2 + m.x11 + m.x12 <= 0)

m.c14 = Constraint(expr= - 10*m.b3 + m.x13 + m.x14 <= 0)

m.c15 = Constraint(expr= - 10*m.b4 + m.x18 <= 0)

m.c16 = Constraint(expr= - 10*m.b5 + m.x21 + m.x23 <= 0)

m.c17 = Constraint(expr= - 10*m.b6 + m.x24 <= 0)

m.c18 = Constraint(expr= - 10*m.b7 + m.x28 <= 0)

m.c19 = Constraint(expr= - 10*m.b8 + m.x30 <= 0)

m.c20 = Constraint(expr= - 10*m.b9 + m.x19 + m.x26 <= 0)

m.c21 = Constraint(expr=exp(m.x12) - m.x11 == 1)

m.c22 = Constraint(expr=   m.x11 == 0)

m.c23 = Constraint(expr=   m.x12 == 0)

m.c24 = Constraint(expr=   m.x35 == 5)

m.c25 = Constraint(expr=exp(0.833333333333333*m.x14) - m.x13 == 1)

m.c26 = Constraint(expr=   m.x13 == 0)

m.c27 = Constraint(expr=   m.x14 == 0)

m.c28 = Constraint(expr=   m.x36 == 8)

m.c29 = Constraint(expr= - m.x17 + 1.5*m.x18 + m.x19 == 0)

m.c30 = Constraint(expr=   m.x18 == 0)

m.c31 = Constraint(expr=   m.x37 == 6)

m.c32 = Constraint(expr=   1.25*m.x21 - m.x22 + 1.25*m.x23 == 0)

m.c33 = Constraint(expr=   m.x21 == 0)

m.c34 = Constraint(expr=   m.x22 == 0)

m.c35 = Constraint(expr=   m.x23 == 0)

m.c36 = Constraint(expr=   m.x38 == 10)

m.c37 = Constraint(expr=   m.x24 - 2*m.x25 == 0)

m.c38 = Constraint(expr=   m.x24 == 0)

m.c39 = Constraint(expr=   m.x25 == 0)

m.c40 = Constraint(expr=   m.x39 == 6)

m.c41 = Constraint(expr=exp(0.666666666666667*m.x29) - m.x28 == 1)

m.c42 = Constraint(expr=   m.x28 == 0)

m.c43 = Constraint(expr=   m.x29 == 0)

m.c44 = Constraint(expr=   m.x40 == 7)

m.c45 = Constraint(expr=exp(m.x31) - m.x30 == 1)

m.c46 = Constraint(expr=   m.x30 == 0)

m.c47 = Constraint(expr=   m.x31 == 0)

m.c48 = Constraint(expr=   m.x41 == 4)

m.c49 = Constraint(expr=exp(m.x27) - m.x19 - m.x26 == 1)

m.c50 = Constraint(expr=   m.x19 == 0)

m.c51 = Constraint(expr=   m.x26 == 0)

m.c52 = Constraint(expr=   m.x27 == 0)

m.c53 = Constraint(expr=   m.x34 == 0)

m.c54 = Constraint(expr=   m.x42 == 5)
