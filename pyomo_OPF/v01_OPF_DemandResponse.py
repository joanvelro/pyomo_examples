import pyomo.environ as pyo 
import math
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Notes
'''
OPF for unbalance three-phase network

The solver does not use per unit values

4 wire cables are transformed into 3x3 primitive matrices with Kron's reduction

Valentin Rigoni - UCD
(\/) (0,,0) (\/)

[1] P. Garcia et all, 'Three-phase power flow calculations using the current injection method', IEEE Transactions on Power Systems, 2000

- Voltages are initialized considering the transformer step up and delta wye connection (30 degrees rotation)
'''

# OPF for demand response
'''
- System has PV
- Demand response from each house
'''

model = pyo.AbstractModel() # Defines an abstract model which data can be imported
## INPUTS
cosphi_constant=1.0
tanphi_constant=-math.tan(math.acos(cosphi_constant))
upper_voltage_limit=1.078
lower_voltage_limit=0.9
main_cable_Alimit=150.0 # Need to add this constraint latter

DR_alpha = 1.0

## Core sets definition ## 

if 1==1:
    model.Buses = pyo.Set(dimen=1)
    model.Phases_abc = pyo.Set(dimen=1) # The model presumes a Kron reduction of the primitive matrices
    model.time = pyo.Set(dimen=1)
    model.data_time = pyo.Set(dimen=1)
    model.Lines = pyo.Set(dimen=1)
    model.Loads = pyo.Set(dimen=1)
    model.PVs = pyo.Set(dimen=1)
    
## Parameters ##
if 1 == 1:
    # Network parameters
    if 1==1:
        model.Bus_G = pyo.Param(model.Buses,model.Buses,model.Phases_abc,model.Phases_abc) # Network conductivity matrix
        model.Bus_B = pyo.Param(model.Buses,model.Buses,model.Phases_abc,model.Phases_abc) # Network susceptance matrix
        model.Connectivity = pyo.Param(model.Buses,model.Buses) # Connectivity matrix 1 if bus k and bus i are connected (zero otherwise)
        model.Lines_k= pyo.Param(model.Lines) # Lines sending buses
        model.Lines_i= pyo.Param(model.Lines) # Lines receiving buses
    # Loads
    if 1==1:
        # Load characteristics
        model.Load_bus_conn= pyo.Param(model.Loads)
        model.Load_phase_conn = pyo.Param(model.Loads)
        model.Load_Vnom = pyo.Param(model.Loads)
        # Demand
        model.PDem= pyo.Param(model.Loads,model.data_time) #W
        model.QDem= pyo.Param(model.Loads,model.data_time) #VA
        model.ZIP_P_P= pyo.Param(model.Loads,model.data_time)
        model.ZIP_P_I= pyo.Param(model.Loads,model.data_time)
        model.ZIP_P_Z= pyo.Param(model.Loads,model.data_time)
        model.ZIP_Q_P= pyo.Param(model.Loads,model.data_time)
        model.ZIP_Q_I= pyo.Param(model.Loads,model.data_time)
        model.ZIP_Q_Z= pyo.Param(model.Loads,model.data_time)
    # PV Generation
    if 1==1:
        model.Percent_Gen= pyo.Param(model.data_time) # Percent generation - function of solar irradiance
        model.PV_sizes= pyo.Param(model.PVs) # Size of each PV in the system
        model.PVs_bus_conn= pyo.Param(model.PVs) # Defines the bus to which each PV is connected
        model.PVs_phase_conn= pyo.Param(model.PVs) # Defines the phase to which each PV is connected
        
    # Variables initialization values
    if 1==1:
        model.Vinitre_pu= pyo.Param(model.Buses,model.Phases_abc,model.data_time) # Values used to initialize the real and imaginary voltages for the solver
        model.Vinitim_pu= pyo.Param(model.Buses,model.Phases_abc,model.data_time)
        model.Vinit= pyo.Param(model.Buses,model.Phases_abc,model.data_time) # Nominal phase voltage for the loads ZIP e.g. 230

        
## PF Variables ## 
if 1==1:
    # Voltages - Real and Imaginary part
    if 1==1:
        # Initialization rules
        def init_Vre_rule(model, k, s, t):
            return model.Vinitre_pu[k,s,t]*model.Vinit[k,s,t]
        def init_Vim_rule(model, k, s, t):
            return model.Vinitim_pu[k,s,t]*model.Vinit[k,s,t]
        # Variables definition
        model.Vre = pyo.Var(model.Buses,model.Phases_abc,model.time, initialize=init_Vre_rule)  # Real and imaginary phase voltages
        model.Vim = pyo.Var(model.Buses,model.Phases_abc,model.time, initialize=init_Vim_rule)    
    # Current flows
    if 1==1:
        model.Iflow_re = pyo.Var(model.Lines,model.Phases_abc,model.time)
        model.Iflow_im = pyo.Var(model.Lines,model.Phases_abc,model.time)
    # PQ flows
    if 1==1:
        model.P_flow_sending = pyo.Var(model.Lines,model.Phases_abc,model.time)
        model.Q_flow_sending = pyo.Var(model.Lines,model.Phases_abc,model.time)
        model.P_flow_receiving = pyo.Var(model.Lines,model.Phases_abc,model.time)
        model.Q_flow_receiving = pyo.Var(model.Lines,model.Phases_abc,model.time)
    # Losses
    if 1==1:
        model.Plosses_line = pyo.Var(model.Lines,model.Phases_abc,model.time)
    # Auxiliary variables for power flow
    if 1==1:
        # Real and imaginary specified current injections for the current missmatch
        model.Isp_re = pyo.Var(model.Buses,model.Phases_abc,model.time, initialize=0.0)
        model.Isp_im = pyo.Var(model.Buses,model.Phases_abc,model.time, initialize=0.0)
    # Dummy variables
    model.Dummy = pyo.Var(initialize=0, within=pyo.NonNegativeReals) # Dummy variable just to have something to put in the objective function
 
## Control Variables ## 
if 1==1:
    # Demand response
    if 1==1:
        model.deltaP = pyo.Var(model.Loads,model.time, initialize=0.0)
    
## Expressions ##
if 1==1:   
    # PV generation
    if 1==1:
        def PV_P_rule(model, pv, t):
            return model.Percent_Gen[t]*model.PV_sizes[pv]*1000*1
        model.PV_P= pyo.Expression(model.PVs,model.time, rule=PV_P_rule) # Active power generation at each bus from PVs
        def PV_Q_rule(model, pv, t): 
            return model.Percent_Gen[t]*model.PV_sizes[pv]*1000*tanphi_constant
        model.PV_Q= pyo.Expression(model.PVs,model.time, rule=PV_Q_rule) # Reactive power generation at each bus from PVs
    # Power injection per node
    if 1==1:
        # Active Power demand ZIP
        def ZIP_P_rule(model, k, s, t):
            return sum(model.deltaP[h,t] + model.PDem[h,t] * (model.ZIP_P_P[h,t] + model.ZIP_P_I[h,t] * ((pyo.sqrt(model.Vre[k,s,t]**2+model.Vim[k,s,t]**2))/model.Load_Vnom[h]) + model.ZIP_P_Z[h,t] * (model.Vre[k,s,t]**2+model.Vim[k,s,t]**2)/(model.Load_Vnom[h]**2)) for h in model.Loads if (model.Load_bus_conn[h]==k and model.Load_phase_conn[h]==s))            
        model.PDem_ZIP = pyo.Expression(model.Buses,model.Phases_abc,model.time,  rule=ZIP_P_rule)
        # Reactive Power demand ZIP
        def ZIP_Q_rule(model, k, s, t):
            return sum(model.QDem[h,t] * (model.ZIP_Q_P[h,t] + model.ZIP_Q_I[h,t] * ((pyo.sqrt(model.Vre[k,s,t]**2+model.Vim[k,s,t]**2))/model.Load_Vnom[h]) + model.ZIP_Q_Z[h,t] * (model.Vre[k,s,t]**2+model.Vim[k,s,t]**2)/(model.Load_Vnom[h]**2)) for h in model.Loads if (model.Load_bus_conn[h]==k and model.Load_phase_conn[h]==s)) 
        model.QDem_ZIP = pyo.Expression(model.Buses,model.Phases_abc,model.time,  rule=ZIP_Q_rule)
        # Active power generation
        def PGen_rule(model, k, s, t):
            return sum(model.PV_P[pv,t] for pv in model.PVs if (model.PVs_bus_conn[pv]==k and model.PVs_phase_conn[pv]==s))            
        model.PGen = pyo.Expression(model.Buses,model.Phases_abc,model.time,  rule=PGen_rule)
        def QGen_rule(model, k, s, t):
            return sum(model.PV_Q[pv,t] for pv in model.PVs if (model.PVs_bus_conn[pv]==k and model.PVs_phase_conn[pv]==s))            
        model.QGen = pyo.Expression(model.Buses,model.Phases_abc,model.time,  rule=QGen_rule)
        # Power injections per node (generation - demand)
        def Psp_rule(model, k, s, t):
            return model.PGen[k,s,t] - model.PDem_ZIP[k,s,t]
        model.Psp = pyo.Expression(model.Buses,model.Phases_abc,model.time, rule=Psp_rule)
        def Qsp_rule(model, k, s, t):
            return model.QGen[k,s,t] - model.QDem_ZIP[k,s,t]
        model.Qsp = pyo.Expression(model.Buses,model.Phases_abc,model.time, rule=Qsp_rule)
    # Current injections
    if 1==1:
        # Specified current injection (from specified PQ injections)
        if 1==1:
            # They are specified as a constraint to as to avoid more complex expressions (simpler to solve.. don't need to divide by the voltage)
            def I_sp_re_rule(model, k, s, t): # Real specified current injection per node and phase
                return model.Psp[k,s,t] == model.Vre[k,s,t] * model.Isp_re[k,s,t] + model.Vim[k,s,t] * model.Isp_im[k,s,t]
            model.Isp_re_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=I_sp_re_rule) 
            def I_sp_im_rule(model, k, s, t): # Imaginary specified current injection per node and phase
                return model.Qsp[k,s,t] == model.Vim[k,s,t] * model.Isp_re[k,s,t] - model.Vre[k,s,t] * model.Isp_im[k,s,t]
            model.Isp_im_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=I_sp_im_rule)
        # Calculated current injection (from voltages and admittances)
        if 1==1:
            # Expressions for currents
            def I_calc_re_rule(model, k, s, t): # Real calculated current injection   
                return sum(sum( model.Bus_G[k,i,s,a]*model.Vre[i,a,t] - model.Bus_B[k,i,s,a]*model.Vim[i,a,t] for a in model.Phases_abc) for i in model.Buses if model.Connectivity[k,i] == 1)
            model.Icalc_re = pyo.Expression(model.Buses,model.Phases_abc,model.time, rule=I_calc_re_rule) 
            def I_calc_im_rule(model, k, s, t): # Imaginary calculated current injection   
                return sum(sum( model.Bus_G[k,i,s,a]*model.Vim[i,a,t] + model.Bus_B[k,i,s,a]*model.Vre[i,a,t] for a in model.Phases_abc) for i in model.Buses if model.Connectivity[k,i] == 1)
            model.Icalc_im = pyo.Expression(model.Buses,model.Phases_abc,model.time, rule=I_calc_im_rule)
    
    ## Power flow variables
    # Current line flows
    if 1==0:
        def I_flow_re_rule(model, l, s, t): # Real line flows
            return model.Iflow_re[l,s,t] == sum(model.Bus_G[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vre[model.Lines_i[l],a,t]-model.Vre[model.Lines_k[l],a,t]) - model.Bus_B[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vim[model.Lines_i[l],a,t]-model.Vim[model.Lines_k[l],a,t]) for a in model.Phases_abc)
        model.Iflow_re_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=I_flow_re_rule) 
        def I_flow_im_rule(model, l, s, t): # Imaginary line flows
            return model.Iflow_im[l,s,t] == sum(model.Bus_B[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vre[model.Lines_i[l],a,t]-model.Vre[model.Lines_k[l],a,t]) + model.Bus_G[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vim[model.Lines_i[l],a,t]-model.Vim[model.Lines_k[l],a,t]) for a in model.Phases_abc)
        model.Iflow_im_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=I_flow_im_rule) 
    # PQ line flows
    if 1==0:
        def P_flow_sending_rule(model, l, s, t): # P sending flow
            return model.P_flow_sending[l,s,t] == model.Vre[model.Lines_k[l],s,t] * model.Iflow_re[l,s,t] + model.Vim[model.Lines_k[l],s,t] * model.Iflow_im[l,s,t]
        model.P_flow_sending_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=P_flow_sending_rule)    
        def P_flow_receiving_rule(model, l, s, t): # P receiving flow
            return model.P_flow_receiving[l,s,t] == -1 *(model.Vre[model.Lines_i[l],s,t] * model.Iflow_re[l,s,t] + model.Vim[model.Lines_i[l],s,t] * model.Iflow_im[l,s,t])  
        model.P_flow_receiving_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=P_flow_receiving_rule)
        def Q_flow_sending_rule(model, l, s, t): # Q sending flow
            return model.Q_flow_sending[l,s,t] == model.Vim[model.Lines_k[l],s,t] * model.Iflow_re[l,s,t] - model.Vre[model.Lines_k[l],s,t] * model.Iflow_im[l,s,t]
        model.Q_flow_sending_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=Q_flow_sending_rule)    
        def Q_flow_receiving_rule(model, l, s, t): # Q receiving flow
            return model.Q_flow_receiving[l,s,t] == -1 *(model.Vim[model.Lines_i[l],s,t] * model.Iflow_re[l,s,t] - model.Vre[model.Lines_i[l],s,t] * model.Iflow_im[l,s,t])  
        model.Q_flow_receiving_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=Q_flow_receiving_rule)
    # Losses
    if 1==0:
        def Plosses_line_rule(model, l, s, t): # P sending flow
            return model.Plosses_line[l,s,t] == model.P_flow_sending[l,s,t] + model.P_flow_receiving[l,s,t]
        model.Plosses_line_con = pyo.Constraint(model.Lines,model.Phases_abc,model.time, rule=Plosses_line_rule)         
    
## Constraints ##
if 1==1:
    # Power flow constraints
    if 1==1:
        # Current mismatch
        def I_delta_re_rule(model, k, s, t): # Real part
            if k=='slack':
                return pyo.Constraint.Skip
            else:
                return model.Isp_re[k,s,t] - model.Icalc_re[k,s,t] == 0
        model.Idelta_re_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=I_delta_re_rule)
        def I_delta_im_rule(model, k, s, t): # Imaginary part
            if k=='slack':
                return pyo.Constraint.Skip
            else:
                return model.Isp_im[k,s,t] - model.Icalc_im[k,s,t] == 0
        model.Idelta_im_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=I_delta_im_rule)
       
        # Slack constraint
        def Slack_re_rule(model, k, s, t): # Real part
            if k=='slack':
                return model.Vre[k,s,t]==model.Vinitre_pu[k,s,t]*model.Vinit[k,s,t]
            else:
                return pyo.Constraint.Skip
        def Slack_im_rule(model, k, s, t): # Imaginary part
            if k=='slack':
                return model.Vim[k,s,t]==model.Vinitim_pu[k,s,t]*model.Vinit[k,s,t]
            else:
                return pyo.Constraint.Skip
        model.Slack_re_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=Slack_re_rule) # Real part
        model.Slack_im_con = pyo.Constraint(model.Buses,model.Phases_abc,model.time, rule=Slack_im_rule) # Imaginary part
    # System state constraints
    if 1==1:
        # Statutory limits
        if 1==1:
            def Voltage_upper_lim_rule(model, h, t): # Again considers that PVs and loads match bus
                return model.Vre[model.Load_bus_conn[h],model.Load_phase_conn[h],t]**2+model.Vim[model.Load_bus_conn[h],model.Load_phase_conn[h],t]**2 <= (upper_voltage_limit * model.Load_Vnom[h])**2 # Writen in terms of the square of the votage to avoid function sqrt() in formulation
            model.Voltage_upper_lim_con = pyo.Constraint(model.Loads,model.time, rule=Voltage_upper_lim_rule) 
            def Voltage_lower_lim_rule(model, h, t): # Again considers that PVs and loads match bus
                return model.Vre[model.Load_bus_conn[h],model.Load_phase_conn[h],t]**2+model.Vim[model.Load_bus_conn[h],model.Load_phase_conn[h],t]**2 >= (lower_voltage_limit * model.Load_Vnom[h])**2 # Writen in terms of the square of the votage to avoid function sqrt() in formulation
            model.Voltage_lower_lim_con = pyo.Constraint(model.Loads,model.time, rule=Voltage_lower_lim_rule) 
    
    # Control variables constraints
    if 1==1:
        # Demand response energy
        if 1==1:
            def deltaP_energy_rule(model, l):
                return sum(model.deltaP[l,t] for t in model.time) == 0
            model.deltaP_energy_con = pyo.Constraint(model.Loads, rule=deltaP_energy_rule)
        # Demand response capabilities
        Loads_set = pd.read_csv('Loads_pyomo.csv')
        Loads_Dem = pd.read_csv('Loads_Dem_pyomo.csv')
        if 1==1:
            # First find max demand
            Pdem_max = pd.DataFrame(index=Loads_set['Loads'].values,columns=['max_kW'])
            for i_load in Loads_set.index: 
                Pdem_max.iloc[i_load]['max_kW'] = Loads_Dem[Loads_Dem['Loads']==Loads_set.loc[i_load,'Loads']]['PDem'].max(0)
            # Limit the max demand response value
            def deltaP_cap_up_rule(model, l ,t):
                return model.deltaP[l,t] <= DR_alpha * float(Pdem_max.loc[l,'max_kW'])
            model.deltaP_cap_up_con = pyo.Constraint(model.Loads,model.time, rule=deltaP_cap_up_rule)
            def deltaP_cap_low_rule(model, l ,t):
                return model.deltaP[l,t] >= -1 * DR_alpha * float(Pdem_max.loc[l,'max_kW'])
            model.deltaP_cap_low_con = pyo.Constraint(model.Loads,model.time, rule=deltaP_cap_low_rule)
        
## Objective function ##
if 1==1:
    # Dummy objective function
    if 1==0:
        model.obj = pyo.Objective(expr=model.Dummy)
    # Min use of DR
    if 1==1:
        def Objective_rule(model):
            return sum(sum(model.deltaP[l,t]**2 for l in model.Loads) for t in model.time)
        model.obj = pyo.Objective(rule=Objective_rule)      

print('Solving OPF')
instance = model.create_instance('Model_data.dat')
if 1==1:
    optimizer = pyo.SolverFactory('ipopt')
    optimizer.options['max_iter'] = 100000
if 1==0:
    optimizer = pyo.SolverFactory('knitroampl')
    optimizer.options["par_numthreads"] = 4

solver_results = optimizer.solve(instance, tee=True)
print('SE solved')



if 1 == 0:

    Bus_set = pd.read_csv('Buses_pyomo.csv')
    Time_sim = pd.read_csv('Time_pyomo.csv')
    Lines_set = pd.read_csv('Lines_pyomo.csv')
    Loads_data_pyomo = pd.read_csv('Loads_data_pyomo.csv')

    Loads_data_pyomo
    # SE Power flow variables
    SE_Bus_Vmag=np.zeros([Time_sim.size,Bus_set.size,3])
    SE_P_line=np.zeros([Time_sim.size,Lines_set.size,3]) # Sending values
    SE_Q_line=np.zeros([Time_sim.size,Lines_set.size,3])
    SE_Ire_line=np.zeros([Time_sim.size,Lines_set.size,3])
    SE_Iim_line=np.zeros([Time_sim.size,Lines_set.size,3])
    SE_Imag_line=np.zeros([Time_sim.size,Lines_set.size,3])
    SE_Losses_line=np.zeros([Time_sim.size,Lines_set.size,3])
    SE_Losses_sign_line=np.zeros([Time_sim.size,Lines_set.size,3])

    for count_t in range(Time_sim.size):
        i_time = Time_sim.loc[count_t,'time']
        # nodal voltages
        for i_bus in range(len(Bus_set)):
            for phase in range(3):
                SE_Bus_Vmag[count_t,i_bus,phase] = math.sqrt(instance.Vre[Bus_set.loc[i_bus,'Buses'],phase+1,i_time].value**2 + instance.Vim[Bus_set.loc[i_bus,'Buses'],phase+1,i_time].value**2)

    """ PLOTS """
    plt.rcParams.update({'font.size': 12})

    # Voltage plot
    load = 'LOAD23'
    fig3, ax = plt.subplots(1, figsize=(10, 6), tight_layout=True)
    bus = Loads_data_pyomo.loc[Loads_data_pyomo['Loads']==load,'Load_bus_conn'].values[0]
    phase = Loads_data_pyomo.loc[Loads_data_pyomo['Loads']==load,'Load_phase_conn'].values[0]
    to_plot_SE=np.zeros([len(Time_sim),3])
    upper_V=np.zeros([len(Time_sim)])
    lower_V=np.zeros([len(Time_sim)])
    for i in range(len(Time_sim)):
        for phase in range(3):
            to_plot_SE[i,phase] = SE_Bus_Vmag[i,Loads_data_pyomo.loc[Loads_data_pyomo['Loads']==load,'Load_bus_conn'].index[0],phase] / (400/math.sqrt(3))
        upper_V[i] = upper_voltage_limit
        lower_V[i] = lower_voltage_limit
    for phase in range(3):
        ax.plot(Time_sim, to_plot_SE[:,phase].T)
    ax.plot(Time_sim, upper_V,dashes=[6, 2])
    #ax.plot(Time_sim, lower_V,dashes=[6, 2])

    # DR usage
    fig3, ax = plt.subplots(1, figsize=(10, 6), tight_layout=True)
    to_plot_SE=np.zeros([len(Time_sim)])
    for i in range(len(Time_sim)):
        i_time = Time_sim.loc[i,'time']
        obj_sum = 0
        for i_load in range(len(Loads_set)):
            obj_sum = obj_sum + instance.deltaP[Loads_set.loc[i_load,'Loads'],i_time].value / 1000.0
        to_plot_SE[i] = obj_sum
    ax.plot(Time_sim, to_plot_SE)

