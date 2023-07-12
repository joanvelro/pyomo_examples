#  MINLP written by GAMS Convert at 05/24/17 13:23:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         99       95        4        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        150      102       48        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        319      232       87        0
# 
#  Reformulation has removed 1 variable and 1 equation


import pyomo.environ as pyo

model = m = pyo.ConcreteModel()

var = pyo.Var
reals = pyo.Reals
m.x1 = var(within=reals, bounds=(10, None), initialize=36.344)
m.x2 = var(within=reals, bounds=(0, None), initialize=1763.656)
m.x3 = var(within=reals, bounds=(1, None), initialize=3.042)
m.x4 = var(within=reals, bounds=(1, None), initialize=10.808)
m.x5 = var(within=reals, bounds=(2, None), initialize=216.161)
m.x6 = var(within=reals, bounds=(1, None), initialize=1.225)
m.x7 = var(within=reals, bounds=(0.1675, None), initialize=2.542)
m.x8 = var(within=reals, bounds=(0, None), initialize=1800)
m.x9 = var(within=reals, bounds=(0, None), initialize=241.731)
m.x10 = var(within=reals, bounds=(0, None), initialize=158.011)
m.x11 = var(within=reals, bounds=(0, None), initialize=88.847)
m.x12 = var(within=reals, bounds=(5, None), initialize=5)
m.x13 = var(within=reals, bounds=(5, None), initialize=18.176)
m.x14 = var(within=reals, bounds=(5, None), initialize=26.048)
m.x15 = var(within=reals, bounds=(5, None), initialize=43.416)
m.x16 = var(within=reals, bounds=(1, None), initialize=241.731)
m.x17 = var(within=reals, bounds=(1, None), initialize=158.011)
m.x18 = var(within=reals, bounds=(1, None), initialize=88.847)
m.x19 = var(within=reals, bounds=(1, None), initialize=36.344)
m.x20 = var(within=reals, bounds=(0, None), initialize=18.176)
m.x21 = var(within=reals, bounds=(0, None), initialize=26.048)
m.x22 = var(within=reals, bounds=(0, None), initialize=43.416)
m.x23 = var(within=reals, bounds=(0, None), initialize=100)
m.x24 = var(within=reals, bounds=(0, None), initialize=1558.269)
m.x25 = var(within=reals, bounds=(0, None), initialize=83.72)
m.x26 = var(within=reals, bounds=(0, None), initialize=69.163)
m.x27 = var(within=reals, bounds=(0, None), initialize=52.503)
m.x28 = var(within=reals, bounds=(0.001, None), initialize=2.956)
m.x29 = var(within=reals, bounds=(0.001, None), initialize=3.317)
m.x30 = var(within=reals, bounds=(0.001, None), initialize=3.737)
m.x31 = var(within=reals, bounds=(0.001, None), initialize=4.248)
m.x32 = var(within=reals, bounds=(1, None), initialize=176.503)
m.x33 = var(within=reals, bounds=(1, None), initialize=10.861)
m.x34 = var(within=reals, bounds=(1, None), initialize=11.542)
m.x35 = var(within=reals, bounds=(1, None), initialize=17.256)
m.x36 = var(within=reals, bounds=(1, None), initialize=1)
m.x37 = var(within=reals, bounds=(1, None), initialize=1)
m.x38 = var(within=reals, bounds=(1, None), initialize=1)
m.x39 = var(within=reals, bounds=(1, None), initialize=1)
m.x40 = var(within=reals, bounds=(1, None), initialize=176.572)
m.x41 = var(within=reals, bounds=(1, None), initialize=154.169)
m.x42 = var(within=reals, bounds=(1, None), initialize=119.85)
m.x43 = var(within=reals, bounds=(1, None), initialize=60.852)
m.x44 = var(within=reals, bounds=(0.01, None), initialize=147.804)
m.x45 = var(within=reals, bounds=(0.01, None), initialize=165.863)
m.x46 = var(within=reals, bounds=(0.01, None), initialize=186.858)
m.x47 = var(within=reals, bounds=(0.01, None), initialize=212.397)
m.x48 = var(within=reals, bounds=(2, 6), initialize=4)
m.x49 = var(within=reals, bounds=(2, 6), initialize=4)
m.x50 = var(within=reals, bounds=(2, 6), initialize=4)
m.x51 = var(within=reals, bounds=(2, 6), initialize=4)
m.x52 = var(within=reals, bounds=(1.26, 6), initialize=3)
m.x53 = var(within=reals, bounds=(1.26, 6), initialize=3)
m.x54 = var(within=reals, bounds=(1.26, 6), initialize=3)
m.x55 = var(within=reals, bounds=(1.26, 6), initialize=3)
m.x56 = var(within=reals, bounds=(1.13E-5, None), initialize=2.1769E-5)
m.x57 = var(within=reals, bounds=(1.13E-5, None), initialize=2.1769E-5)
m.x58 = var(within=reals, bounds=(1.13E-5, None), initialize=2.1769E-5)
m.x59 = var(within=reals, bounds=(1.13E-5, None), initialize=2.1769E-5)
m.x60 = var(within=reals, bounds=(2.9E-7, None), initialize=1.299)
m.x61 = var(within=reals, bounds=(2.9E-7, None), initialize=1.642)
m.x62 = var(within=reals, bounds=(2.9E-7, None), initialize=2.167)
m.x63 = var(within=reals, bounds=(2.9E-7, None), initialize=3.069)
m.x64 = var(within=reals, bounds=(3E-7, None), initialize=1.3)
m.x65 = var(within=reals, bounds=(3E-7, None), initialize=1.643)
m.x66 = var(within=reals, bounds=(3E-7, None), initialize=2.169)
m.x67 = var(within=reals, bounds=(3E-7, None), initialize=3.072)
m.x68 = var(within=reals, bounds=(3E-7, None), initialize=0.001)
m.x69 = var(within=reals, bounds=(3E-7, None), initialize=0.001)
m.x70 = var(within=reals, bounds=(3E-7, None), initialize=0.002)
m.x71 = var(within=reals, bounds=(3E-7, None), initialize=0.002)
m.x72 = var(within=reals, bounds=(0.05, None), initialize=8.825)
m.x73 = var(within=reals, bounds=(0.05, None), initialize=0.543)
m.x74 = var(within=reals, bounds=(0.05, None), initialize=0.577)
m.x75 = var(within=reals, bounds=(0.05, None), initialize=0.863)
m.x76 = var(within=reals, bounds=(0, None), initialize=1121.796)
m.x77 = var(within=reals, bounds=(0, None), initialize=1121.796)
m.x78 = var(within=reals, bounds=(0, None), initialize=1121.796)
m.x79 = var(within=reals, bounds=(0, None), initialize=1121.796)
m.x80 = var(within=reals, bounds=(0, None), initialize=198000)
m.x81 = var(within=reals, bounds=(0, None), initialize=12183.696)
m.x82 = var(within=reals, bounds=(0, None), initialize=12947.373)
m.x83 = var(within=reals, bounds=(0, None), initialize=19357.594)
m.x84 = var(within=reals, bounds=(0, None), initialize=0.2)
m.x85 = var(within=reals, bounds=(0, None), initialize=0.027)
m.x86 = var(within=reals, bounds=(0, None), initialize=0.018)
m.x87 = var(within=reals, bounds=(0, None), initialize=0.01)
m.x88 = var(within=reals, bounds=(0, None), initialize=1.1)
m.x89 = var(within=reals, bounds=(0, None), initialize=0.068)
m.x90 = var(within=reals, bounds=(0, None), initialize=0.072)
m.x91 = var(within=reals, bounds=(0, None), initialize=0.108)
m.x92 = var(within=reals, bounds=(0, None), initialize=2.063)
m.x93 = var(within=reals, bounds=(0, None), initialize=0.15)
m.x94 = var(within=reals, bounds=(0, None), initialize=0.142)
m.x95 = var(within=reals, bounds=(0, None), initialize=0.186)
binary = pyo.Binary
m.b96 = var(within=binary, bounds=(0, 1), initialize=0)
m.b97 = var(within=binary, bounds=(0, 1), initialize=0)
m.b98 = var(within=binary, bounds=(0, 1), initialize=0)
m.b99 = var(within=binary, bounds=(0, 1), initialize=0)
m.b100 = var(within=binary, bounds=(0, 1), initialize=0)
m.b101 = var(within=binary, bounds=(0, 1), initialize=0)
m.b102 = var(within=binary, bounds=(0, 1), initialize=0)
m.b103 = var(within=binary, bounds=(0, 1), initialize=0)
m.b104 = var(within=binary, bounds=(0, 1), initialize=0)
m.b105 = var(within=binary, bounds=(0, 1), initialize=0)
m.b106 = var(within=binary, bounds=(0, 1), initialize=0)
m.b107 = var(within=binary, bounds=(0, 1), initialize=0)
m.b108 = var(within=binary, bounds=(0, 1), initialize=0)
m.b109 = var(within=binary, bounds=(0, 1), initialize=0)
m.b110 = var(within=binary, bounds=(0, 1), initialize=0)
m.b111 = var(within=binary, bounds=(0, 1), initialize=0)
m.b112 = var(within=binary, bounds=(0, 1), initialize=0)
m.b113 = var(within=binary, bounds=(0, 1), initialize=0)
m.b114 = var(within=binary, bounds=(0, 1), initialize=0)
m.b115 = var(within=binary, bounds=(0, 1), initialize=0)
m.b116 = var(within=binary, bounds=(0, 1), initialize=0)
m.b117 = var(within=binary, bounds=(0, 1), initialize=0)
m.b118 = var(within=binary, bounds=(0, 1), initialize=0)
m.b119 = var(within=binary, bounds=(0, 1), initialize=0)
m.b120 = var(within=binary, bounds=(0, 1), initialize=0)
m.b121 = var(within=binary, bounds=(0, 1), initialize=0)
m.b122 = var(within=binary, bounds=(0, 1), initialize=0)
m.b123 = var(within=binary, bounds=(0, 1), initialize=0)
m.b124 = var(within=binary, bounds=(0, 1), initialize=0)
m.b125 = var(within=binary, bounds=(0, 1), initialize=0)
m.b126 = var(within=binary, bounds=(0, 1), initialize=0)
m.b127 = var(within=binary, bounds=(0, 1), initialize=0)
m.b128 = var(within=binary, bounds=(0, 1), initialize=0)
m.b129 = var(within=binary, bounds=(0, 1), initialize=0)
m.b130 = var(within=binary, bounds=(0, 1), initialize=0)
m.b131 = var(within=binary, bounds=(0, 1), initialize=0)
m.b132 = var(within=binary, bounds=(0, 1), initialize=0)
m.b133 = var(within=binary, bounds=(0, 1), initialize=0)
m.b134 = var(within=binary, bounds=(0, 1), initialize=0)
m.b135 = var(within=binary, bounds=(0, 1), initialize=0)
m.b136 = var(within=binary, bounds=(0, 1), initialize=0)
m.b137 = var(within=binary, bounds=(0, 1), initialize=0)
m.b138 = var(within=binary, bounds=(0, 1), initialize=0)
m.b139 = var(within=binary, bounds=(0, 1), initialize=0)
m.b140 = var(within=binary, bounds=(0, 1), initialize=0)
m.b141 = var(within=binary, bounds=(0, 1), initialize=0)
m.b142 = var(within=binary, bounds=(0, 1), initialize=0)
m.b143 = var(within=binary, bounds=(0, 1), initialize=0)
m.x145 = var(within=reals, bounds=(0, None), initialize=0)
m.x146 = var(within=reals, bounds=(0, None), initialize=0)
m.x147 = var(within=reals, bounds=(0, None), initialize=0)
m.x148 = var(within=reals, bounds=(0, None), initialize=0)
m.x149 = var(within=reals, bounds=(0, None), initialize=0)
m.x150 = var(within=reals, bounds=(0, None), initialize=0)


m.obj = pyo.Objective(expr=m.x145 + m.x146 + m.x147 + m.x148 + m.x149 + m.x150 + 3271.22725820856, sense=pyo.minimize)

constraint = pyo.Constraint
m.c2 = constraint(expr=m.x8 == 1800)

m.c3 = constraint(expr=m.x12 == 5)

m.c4 = constraint(expr=m.x2 - m.x24 - m.x25 - m.x26 - m.x27 == 0)

m.c5 = constraint(expr=-(m.x28 * m.x24 + m.x29 * m.x25 + m.x30 * m.x26 + m.x31 * m.x27) / m.x2 + m.x3 == 0)

m.c6 = constraint(expr=m.x1 - m.x19 == 0)

m.c7 = constraint(expr=m.x23 == 100)

m.c8 = constraint(expr=m.x9 - m.x16 == 0)

m.c9 = constraint(expr=m.x13 - m.x20 == 0)

m.c10 = constraint(expr=m.x5 - m.x32 - m.x33 - m.x34 - m.x35 == 0)

m.c11 = constraint(expr=m.x6 - m.x36 - m.x37 - m.x38 - m.x39 == 0)

m.c12 = constraint(expr=m.x7 - m.x92 - m.x93 - m.x94 - m.x95 == 0)

m.c13 = constraint(expr=m.x4 - m.x72 - m.x73 - m.x74 - m.x75 == 0)

m.c14 = constraint(expr=m.x10 - m.x17 == 0)

m.c15 = constraint(expr=m.x14 - m.x21 == 0)

m.c16 = constraint(expr=m.x11 - m.x18 == 0)

m.c17 = constraint(expr=m.x15 - m.x22 == 0)

m.c18 = constraint(expr=2.77777777777778e-7 * m.x40 / pyo.log((m.x44 - m.x28) / (m.x20 - m.x28)) - m.x56 == 0)

m.c19 = constraint(expr=2.77777777777778e-7 * m.x41 / pyo.log((m.x45 - m.x29) / (m.x21 - m.x29)) - m.x57 == 0)

m.c20 = constraint(expr=2.77777777777778e-7 * m.x42 / pyo.log((m.x46 - m.x30) / (m.x22 - m.x30)) - m.x58 == 0)

m.c21 = constraint(expr=2.77777777777778e-7 * m.x43 / pyo.log((m.x47 - m.x31) / (m.x23 - m.x31)) - m.x59 == 0)

m.c22 = constraint(expr=50 * m.x28 - m.x44 == 0)

m.c23 = constraint(expr=50 * m.x29 - m.x45 == 0)

m.c24 = constraint(expr=50 * m.x30 - m.x46 == 0)

m.c25 = constraint(expr=50 * m.x31 - m.x47 == 0)

m.c26 = constraint(expr=m.x40 - 65.38084341288 * m.x48 + 65.38084341288 * m.x60 == 0)

m.c27 = constraint(expr=m.x41 - 65.38084341288 * m.x49 + 65.38084341288 * m.x61 == 0)

m.c28 = constraint(expr=m.x42 - 65.38084341288 * m.x50 + 65.38084341288 * m.x62 == 0)

m.c29 = constraint(expr=m.x43 - 65.38084341288 * m.x51 + 65.38084341288 * m.x63 == 0)

m.c30 = constraint(expr=- m.x60 + m.x64 - m.x68 == 0)

m.c31 = constraint(expr=- m.x61 + m.x65 - m.x69 == 0)

m.c32 = constraint(expr=- m.x62 + m.x66 - m.x70 == 0)

m.c33 = constraint(expr=- m.x63 + m.x67 - m.x71 == 0)

m.c34 = constraint(
    expr=-1e-5 * (12.09 * m.x44 ** 2 + 3.66 * m.x44 - 0.08 * m.x44 ** 3 + 0.0002592 * m.x44 ** 4) + m.x64 == 0)

m.c35 = constraint(
    expr=-1e-5 * (12.09 * m.x45 ** 2 + 3.66 * m.x45 - 0.08 * m.x45 ** 3 + 0.0002592 * m.x45 ** 4) + m.x65 == 0)

m.c36 = constraint(
    expr=-1e-5 * (12.09 * m.x46 ** 2 + 3.66 * m.x46 - 0.08 * m.x46 ** 3 + 0.0002592 * m.x46 ** 4) + m.x66 == 0)

m.c37 = constraint(
    expr=-1e-5 * (12.09 * m.x47 ** 2 + 3.66 * m.x47 - 0.08 * m.x47 ** 3 + 0.0002592 * m.x47 ** 4) + m.x67 == 0)

m.c38 = constraint(
    expr=-1e-5 * (12.09 * m.x28 ** 2 + 3.66 * m.x28 - 0.08 * m.x28 ** 3 + 0.0002592 * m.x28 ** 4) + m.x68 == 0)

m.c39 = constraint(
    expr=-1e-5 * (12.09 * m.x29 ** 2 + 3.66 * m.x29 - 0.08 * m.x29 ** 3 + 0.0002592 * m.x29 ** 4) + m.x69 == 0)

m.c40 = constraint(
    expr=-1e-5 * (12.09 * m.x30 ** 2 + 3.66 * m.x30 - 0.08 * m.x30 ** 3 + 0.0002592 * m.x30 ** 4) + m.x70 == 0)

m.c41 = constraint(
    expr=-1e-5 * (12.09 * m.x31 ** 2 + 3.66 * m.x31 - 0.08 * m.x31 ** 3 + 0.0002592 * m.x31 ** 4) + m.x71 == 0)

m.c42 = constraint(expr=-1.13572384718704e-8 * (7936.50793650794 * m.x52) ** 0.75 + m.x56 == 0)

m.c43 = constraint(expr=-1.13572384718704e-8 * (7936.50793650794 * m.x53) ** 0.75 + m.x57 == 0)

m.c44 = constraint(expr=-1.13572384718704e-8 * (7936.50793650794 * m.x54) ** 0.75 + m.x58 == 0)

m.c45 = constraint(expr=-1.13572384718704e-8 * (7936.50793650794 * m.x55) ** 0.75 + m.x59 == 0)

m.c46 = constraint(expr=- m.x8 + m.x16 + m.x24 == 0)

m.c47 = constraint(expr=- m.x9 + m.x17 + m.x25 == 0)

m.c48 = constraint(expr=- m.x10 + m.x18 + m.x26 == 0)

m.c49 = constraint(expr=- m.x11 + m.x19 + m.x27 == 0)

m.c50 = constraint(expr=m.x12 * m.x8 - (m.x20 * m.x16 + m.x28 * m.x24) == 0)

m.c51 = constraint(expr=m.x13 * m.x9 - (m.x21 * m.x17 + m.x29 * m.x25) == 0)

m.c52 = constraint(expr=m.x14 * m.x10 - (m.x22 * m.x18 + m.x30 * m.x26) == 0)

m.c53 = constraint(expr=m.x15 * m.x11 - (m.x23 * m.x19 + m.x31 * m.x27) == 0)

m.c54 = constraint(expr=-2.77777777777778e-5 * m.x48 * m.x8 + m.x84 == 0)

m.c55 = constraint(expr=-2.77777777777778e-5 * m.x49 * m.x9 + m.x85 == 0)

m.c56 = constraint(expr=-2.77777777777778e-5 * m.x50 * m.x10 + m.x86 == 0)

m.c57 = constraint(expr=-2.77777777777778e-5 * m.x51 * m.x11 + m.x87 == 0)

m.c58 = constraint(expr=-m.x24 / m.x40 + m.x72 == 0)

m.c59 = constraint(expr=-m.x25 / m.x41 + m.x73 == 0)

m.c60 = constraint(expr=-m.x26 / m.x42 + m.x74 == 0)

m.c61 = constraint(expr=-m.x27 / m.x43 + m.x75 == 0)

m.c62 = constraint(expr=m.x32 - 20 * m.x72 == 0)

m.c63 = constraint(expr=m.x33 - 20 * m.x73 == 0)

m.c64 = constraint(expr=m.x34 - 20 * m.x74 == 0)

m.c65 = constraint(expr=m.x35 - 20 * m.x75 == 0)

m.c66 = constraint(expr=- 373.932 * m.x52 + m.x76 == 0)

m.c67 = constraint(expr=- 373.932 * m.x53 + m.x77 == 0)

m.c68 = constraint(expr=- 373.932 * m.x54 + m.x78 == 0)

m.c69 = constraint(expr=- 373.932 * m.x55 + m.x79 == 0)

m.c70 = constraint(expr=-m.x32 * m.x76 + m.x80 == 0)

m.c71 = constraint(expr=-m.x33 * m.x77 + m.x81 == 0)

m.c72 = constraint(expr=-m.x34 * m.x78 + m.x82 == 0)

m.c73 = constraint(expr=-m.x35 * m.x79 + m.x83 == 0)

m.c74 = constraint(expr=- 5.55555555555556E-6 * m.x80 + m.x88 == 0)

m.c75 = constraint(expr=- 5.55555555555556E-6 * m.x81 + m.x89 == 0)

m.c76 = constraint(expr=- 5.55555555555556E-6 * m.x82 + m.x90 == 0)

m.c77 = constraint(expr=- 5.55555555555556E-6 * m.x83 + m.x91 == 0)

m.c78 = constraint(expr=- 1.58730158730159 * m.x84 - 1.58730158730159 * m.x88 + m.x92 == 0)

m.c79 = constraint(expr=- 1.58730158730159 * m.x85 - 1.58730158730159 * m.x89 + m.x93 == 0)

m.c80 = constraint(expr=- 1.58730158730159 * m.x86 - 1.58730158730159 * m.x90 + m.x94 == 0)

m.c81 = constraint(expr=- 1.58730158730159 * m.x87 - 1.58730158730159 * m.x91 + m.x95 == 0)

m.c82 = constraint(expr=m.x36 - 0.909090909090909 * m.x88 >= 0)

m.c83 = constraint(expr=m.x37 - 0.909090909090909 * m.x89 >= 0)

m.c84 = constraint(expr=m.x38 - 0.909090909090909 * m.x90 >= 0)

m.c85 = constraint(expr=m.x39 - 0.909090909090909 * m.x91 >= 0)

m.c86 = constraint(expr=m.x32 - m.b96 - 2 * m.b100 - 4 * m.b104 - 8 * m.b108 - 16 * m.b112 - 32 * m.b116 - 64 * m.b120
                        - 128 * m.b124 == 0)

m.c87 = constraint(expr=m.x33 - m.b97 - 2 * m.b101 - 4 * m.b105 - 8 * m.b109 - 16 * m.b113 - 32 * m.b117 - 64 * m.b121
                        - 128 * m.b125 == 0)

m.c88 = constraint(expr=m.x34 - m.b98 - 2 * m.b102 - 4 * m.b106 - 8 * m.b110 - 16 * m.b114 - 32 * m.b118 - 64 * m.b122
                        - 128 * m.b126 == 0)

m.c89 = constraint(expr=m.x35 - m.b99 - 2 * m.b103 - 4 * m.b107 - 8 * m.b111 - 16 * m.b115 - 32 * m.b119 - 64 * m.b123
                        - 128 * m.b127 == 0)

m.c90 = constraint(expr=m.x36 - m.b128 - 2 * m.b132 - 4 * m.b136 - 8 * m.b140 == 0)

m.c91 = constraint(expr=m.x37 - m.b129 - 2 * m.b133 - 4 * m.b137 - 8 * m.b141 == 0)

m.c92 = constraint(expr=m.x38 - m.b130 - 2 * m.b134 - 4 * m.b138 - 8 * m.b142 == 0)

m.c93 = constraint(expr=m.x39 - m.b131 - 2 * m.b135 - 4 * m.b139 - 8 * m.b143 == 0)

m.c94 = constraint(expr=m.x145 == 5047.03634123606)

m.c95 = constraint(expr=- 292.07386234005 * m.x6 + m.x146 == 0)

m.c96 = constraint(expr=- 2103.94993266178 * m.x7 + m.x149 == 0)

m.c97 = constraint(expr=- 45.7380420143865 * m.x2 + m.x147 == 0)

m.c98 = constraint(expr=-4.57380420143865 * m.x2 * m.x3 + m.x148 == 0)

m.c99 = constraint(expr=- 764.973851088085 * m.x4 + m.x150 == 0)

opt = pyo.SolverFactory("Bonmin")
opt_params = dict()
opt.options["allowable_fraction_gap"] = opt_params.get('allowable_fraction_gap', 0.0001)

results = opt.solve(model, tee=True)  # tee=True for details
