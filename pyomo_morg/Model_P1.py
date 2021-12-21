#  MINLP written by GAMS Convert at 05/24/17 13:24:11
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         28       10        0       18        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         28        7       21        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        103       76       27        0
# 
#  Reformulation has removed 1 variable and 1 equation


from pyomo.environ import *

model = m = ConcreteModel()


m.x1 = Var(within=Reals,bounds=(1,250),initialize=1)
m.x2 = Var(within=Reals,bounds=(1,250),initialize=1)
m.x3 = Var(within=Reals,bounds=(1,250),initialize=1)
m.x4 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x5 = Var(within=Reals,bounds=(0,None),initialize=0)
m.x6 = Var(within=Reals,bounds=(0,None),initialize=0)
m.b7 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b8 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b9 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b10 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b11 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b12 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b13 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b14 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b15 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b16 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b17 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b18 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b19 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b20 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b21 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b22 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b23 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b24 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b25 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b26 = Var(within=Binary,bounds=(0,1),initialize=0)
m.b27 = Var(within=Binary,bounds=(0,1),initialize=0)

m.obj = Objective(expr=((0.013 + 0.0024*m.x4/m.x1)*m.x1 + 10*m.x1 + 0.06*m.x4/m.x1)*m.b7 + ((0.01 + 0.002*m.x5/m.x2)*
                       m.x2 + 10*m.x2 + 0.06*m.x5/m.x2)*m.b8 + ((0.014 + 0.0028*m.x6/m.x3)*m.x3 + 10*m.x3 + 0.06*m.x6/
                       m.x3)*m.b9 + 100*m.b7 + 100*m.b8 + 100*m.b9 + 0.2352*(sqrt(6300*m.b10 + 17500*m.b13 + 4375*m.b16
                        + 44800*m.b19 + 4375*m.b22 + 44800*m.b25) + sqrt(6300*m.b11 + 17500*m.b14 + 4375*m.b17 + 44800*
                       m.b20 + 4375*m.b23 + 44800*m.b26) + sqrt(6300*m.b12 + 17500*m.b15 + 4375*m.b18 + 44800*m.b21 + 
                       4375*m.b24 + 44800*m.b27)) + 9.5*m.b10 + 475*m.b11 + 684*m.b12 + 31.4*m.b13 + 533.8*m.b14 + 518.1
                       *m.b15 + 41.4*m.b16 + 9.2*m.b17 + 119.6*m.b18 + 514.8*m.b19 + 58.5*m.b20 + 304.2*m.b21 + 285*
                       m.b22 + 337.5*m.b23 + 22.5*m.b24 + 1612.8*m.b25 + 1094.4*m.b26 + 38.4*m.b27, sense=minimize)

m.c2 = Constraint(expr=   m.b10 + m.b11 + m.b12 == 1)

m.c3 = Constraint(expr=   m.b13 + m.b14 + m.b15 == 1)

m.c4 = Constraint(expr=   m.b16 + m.b17 + m.b18 == 1)

m.c5 = Constraint(expr=   m.b19 + m.b20 + m.b21 == 1)

m.c6 = Constraint(expr=   m.b22 + m.b23 + m.b24 == 1)

m.c7 = Constraint(expr=   m.b25 + m.b26 + m.b27 == 1)

m.c8 = Constraint(expr= - m.b7 + m.b10 <= 0)

m.c9 = Constraint(expr= - m.b8 + m.b11 <= 0)

m.c10 = Constraint(expr= - m.b9 + m.b12 <= 0)

m.c11 = Constraint(expr= - m.b7 + m.b13 <= 0)

m.c12 = Constraint(expr= - m.b8 + m.b14 <= 0)

m.c13 = Constraint(expr= - m.b9 + m.b15 <= 0)

m.c14 = Constraint(expr= - m.b7 + m.b16 <= 0)

m.c15 = Constraint(expr= - m.b8 + m.b17 <= 0)

m.c16 = Constraint(expr= - m.b9 + m.b18 <= 0)

m.c17 = Constraint(expr= - m.b7 + m.b19 <= 0)

m.c18 = Constraint(expr= - m.b8 + m.b20 <= 0)

m.c19 = Constraint(expr= - m.b9 + m.b21 <= 0)

m.c20 = Constraint(expr= - m.b7 + m.b22 <= 0)

m.c21 = Constraint(expr= - m.b8 + m.b23 <= 0)

m.c22 = Constraint(expr= - m.b9 + m.b24 <= 0)

m.c23 = Constraint(expr= - m.b7 + m.b25 <= 0)

m.c24 = Constraint(expr= - m.b8 + m.b26 <= 0)

m.c25 = Constraint(expr= - m.b9 + m.b27 <= 0)

m.c26 = Constraint(expr=   m.x4 - 23750*m.b10 - 39250*m.b13 - 11500*m.b16 - 58500*m.b19 - 18750*m.b22 - 48000*m.b25
                         == 0)

m.c27 = Constraint(expr=   m.x5 - 23750*m.b11 - 39250*m.b14 - 11500*m.b17 - 58500*m.b20 - 18750*m.b23 - 48000*m.b26
                         == 0)

m.c28 = Constraint(expr=   m.x6 - 23750*m.b12 - 39250*m.b15 - 11500*m.b18 - 58500*m.b21 - 18750*m.b24 - 48000*m.b27
                         == 0)
