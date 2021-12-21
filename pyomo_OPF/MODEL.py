from __future__ import division
from pyomo.environ import *

# Notes
'''
OPF for unbalance three-phase network

The solver does not use per unit values

4 wire cables are transformed into 3x3 primitive matrices with Kron's reduction

Author: Valentin Rigoni - valentin.rigoni@ucdconnect.ie - University College Dublin
(\/) (0,,0) (\/)

[1] P. Garcia et all, 'Three-phase power flow calculations using the current injection method', IEEE Transactions on Power Systems, 2000
'''

# New version v03
'''
- Includes solar irradiance
- Includes some PV control capabilities
    * All PVs behave uniformly
- Voltages are initialized considering the transformer step up and delta wye connection (30 degrees rotation)
'''

# IMPROVEMENTS
'''
- Add OLTC capabilities (change on Y matrix composition?)
- Can be upgraded to 4wire quite easily
'''

model = AbstractModel() # Defines an abstract model which data can be imported

## INPUTS
cosphi_limit=0.9
capacitive_PV_limit=-tan(acos(cosphi_limit))
inductive_PV_limit=0.0
min_PVoutput=0.0
Inverter_oversize=1.1
upper_voltage_limit=1.08
lower_voltage_limit=0.9
main_cable_Alimit=999999999 # Need to add this constraint latter

## Core sets definition ##
if 1==1:
    model.Buses = Set(dimen=1)
    model.Phases_abc = Set(dimen=1) # The model presumes a Kron reduction of the primitive matrices
    model.time = Set(dimen=1)
    model.Lines = Set(dimen=1)
    model.PVs = Set(dimen=1)
    model.Loads = Set(dimen=1)
    
## Parameters ##
if 1==1:
    # Timestep
    model.Timestep= Param() # in minutes
    # Network parameters
    if 1==1:
        model.Bus_G = Param(model.Buses,model.Buses,model.Phases_abc,model.Phases_abc) # Network conductivity matrix
        model.Bus_B = Param(model.Buses,model.Buses,model.Phases_abc,model.Phases_abc) # Network susceptance matrix
        model.Connectivity = Param(model.Buses,model.Buses) # Connectivity matrix 1 if bus k and bus i are connected (zero otherwise)
        model.Lines_k= Param(model.Lines) # Lines sending buses
        model.Lines_i= Param(model.Lines) # Lines receiving buses
    # Loads
    if 1==1:
        # Load characteristics
        model.Load_bus_conn= Param(model.Loads)
        model.Load_phase_conn = Param(model.Loads)
        model.Load_Vnom = Param(model.Loads)
        # Demand
        model.PDem= Param(model.Loads,model.time) #W
        model.QDem= Param(model.Loads,model.time) #VA
        model.ZIP_P_P= Param(model.Loads,model.time)
        model.ZIP_P_I= Param(model.Loads,model.time)
        model.ZIP_P_Z= Param(model.Loads,model.time)
        model.ZIP_Q_P= Param(model.Loads,model.time)
        model.ZIP_Q_I= Param(model.Loads,model.time)
        model.ZIP_Q_Z= Param(model.Loads,model.time)
    # PV Generation
    if 1==1:
        model.Percent_Gen= Param(model.time) # Percent generation - function of solar irradiance
        model.PV_sizes= Param(model.PVs) # Size of each PV in the system
        model.PVs_bus_conn= Param(model.PVs) # Defines the bus to which each PV is connected
        model.PVs_phase_conn= Param(model.PVs) # Defines the phase to which each PV is connected
    # Variables initialization values
    if 1==1:
        model.Vinitre_pu= Param(model.Buses,model.Phases_abc,model.time) # Values used to initialize the real and imaginary voltages for the solver
        model.Vinitim_pu= Param(model.Buses,model.Phases_abc,model.time)
        model.Vinit= Param(model.Buses,model.Phases_abc,model.time) # Nominal phase voltage for the loads ZIP e.g. 230
        
## Variables ## 
if 1==1:
    # Voltages - Real and Imaginary part
    if 1==1:
        # Initialization rules
        def init_Vre_rule(model, k, s, t):
            return model.Vinitre_pu[k,s,t]*model.Vinit[k,s,t]
        def init_Vim_rule(model, k, s, t):
            return model.Vinitim_pu[k,s,t]*model.Vinit[k,s,t]
        # Variables definition
        model.Vre = Var(model.Buses,model.Phases_abc,model.time, initialize=init_Vre_rule)  # Real and imaginary phase voltages
        model.Vim = Var(model.Buses,model.Phases_abc,model.time, initialize=init_Vim_rule) 
    # Auxiliary variables for power flow
    if 1==1:
        # Real and imaginary specified current injections for the current missmatch
        model.Isp_re = Var(model.Buses,model.Phases_abc,model.time, initialize=0.0)
        model.Isp_im = Var(model.Buses,model.Phases_abc,model.time, initialize=0.0)
    # Dummy variables
    model.Dummy = Var(initialize=0, within=NonNegativeReals) # Dummy variable just to have something to put in the objective function
    
## Expressions ##
if 1==1:     
    # PV generation
    if 1==1:
        def PV_P_rule(model, pv, t):
            return model.Percent_Gen[t]*model.PV_sizes[pv]*1000*1
        model.PV_P= Expression(model.PVs,model.time, rule=PV_P_rule) # Active power generation at each bus from PVs
        def PV_Q_rule(model, pv, t): 
            return model.Percent_Gen[t]*model.PV_sizes[pv]*1000*-0.2
        model.PV_Q= Expression(model.PVs,model.time, rule=PV_Q_rule) # Reactive power generation at each bus from PVs
    # Power injection per node
    if 1==1:
        # Active Power demand ZIP
        def ZIP_P_rule(model, k, s, t):
            return sum(model.PDem[h,t] * (model.ZIP_P_P[h,t] + model.ZIP_P_I[h,t] * ((sqrt(model.Vre[k,s,t]**2+model.Vim[k,s,t]**2))/model.Load_Vnom[h]) + model.ZIP_P_Z[h,t] * (model.Vre[k,s,t]**2+model.Vim[k,s,t]**2)/(model.Load_Vnom[h]**2)) for h in model.Loads if (model.Load_bus_conn[h]==k and model.Load_phase_conn[h]==s))            
        model.PDem_ZIP = Expression(model.Buses,model.Phases_abc,model.time,  rule=ZIP_P_rule)
        # Reactive Power demand ZIP
        def ZIP_Q_rule(model, k, s, t):
            return sum(model.QDem[h,t] * (model.ZIP_Q_P[h,t] + model.ZIP_Q_I[h,t] * ((sqrt(model.Vre[k,s,t]**2+model.Vim[k,s,t]**2))/model.Load_Vnom[h]) + model.ZIP_Q_Z[h,t] * (model.Vre[k,s,t]**2+model.Vim[k,s,t]**2)/(model.Load_Vnom[h]**2)) for h in model.Loads if (model.Load_bus_conn[h]==k and model.Load_phase_conn[h]==s)) 
        model.QDem_ZIP = Expression(model.Buses,model.Phases_abc,model.time,  rule=ZIP_Q_rule)
        # Active power generation
        def PGen_rule(model, k, s, t):
            return sum(model.PV_P[pv,t] for pv in model.PVs if (model.PVs_bus_conn[pv]==k and model.PVs_phase_conn[pv]==s))            
        model.PGen = Expression(model.Buses,model.Phases_abc,model.time,  rule=PGen_rule)
        def QGen_rule(model, k, s, t):
            return sum(model.PV_Q[pv,t] for pv in model.PVs if (model.PVs_bus_conn[pv]==k and model.PVs_phase_conn[pv]==s))            
        model.QGen = Expression(model.Buses,model.Phases_abc,model.time,  rule=QGen_rule)
        # Power injections per node (generation - demand)
        def Psp_rule(model, k, s, t):
            return model.PGen[k,s,t] - model.PDem_ZIP[k,s,t]
        model.Psp = Expression(model.Buses,model.Phases_abc,model.time, rule=Psp_rule)
        def Qsp_rule(model, k, s, t):
            return model.QGen[k,s,t] - model.QDem_ZIP[k,s,t]
        model.Qsp = Expression(model.Buses,model.Phases_abc,model.time, rule=Qsp_rule)
    # Current injections
    if 1==1:
        # Specified current injection (from specified PQ injections)
        if 1==1:
            # They are specified as a constraint to as to avoid more complex expressions (simpler to solve.. don't need to divide by the voltage)
            def I_sp_re_rule(model, k, s, t): # Real specified current injection per node and phase
                return model.Psp[k,s,t] == model.Vre[k,s,t] * model.Isp_re[k,s,t] + model.Vim[k,s,t] * model.Isp_im[k,s,t]
            model.Isp_re_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=I_sp_re_rule) 
            def I_sp_im_rule(model, k, s, t): # Imaginary specified current injection per node and phase
                return model.Qsp[k,s,t] == model.Vim[k,s,t] * model.Isp_re[k,s,t] - model.Vre[k,s,t] * model.Isp_im[k,s,t]
            model.Isp_im_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=I_sp_im_rule)
        # Calculated current injection (from voltages and admittances)
        if 1==1:
            # Expressions for currents
            def I_calc_re_rule(model, k, s, t): # Real calculated current injection   
                return sum(sum( model.Bus_G[k,i,s,a]*model.Vre[i,a,t] - model.Bus_B[k,i,s,a]*model.Vim[i,a,t] for a in model.Phases_abc) for i in model.Buses if model.Connectivity[k,i] == 1)
            model.Icalc_re = Expression(model.Buses,model.Phases_abc,model.time, rule=I_calc_re_rule) 
            def I_calc_im_rule(model, k, s, t): # Imaginary calculated current injection   
                return sum(sum( model.Bus_G[k,i,s,a]*model.Vim[i,a,t] + model.Bus_B[k,i,s,a]*model.Vre[i,a,t] for a in model.Phases_abc) for i in model.Buses if model.Connectivity[k,i] == 1)
            model.Icalc_im = Expression(model.Buses,model.Phases_abc,model.time, rule=I_calc_im_rule) 
    # Current line flows
    if 1==1:
        def I_flow_re_rule(model, l, s, t): # Real line flows
            return sum(model.Bus_G[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vre[model.Lines_i[l],a,t]-model.Vre[model.Lines_k[l],a,t]) - model.Bus_B[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vim[model.Lines_i[l],a,t]-model.Vim[model.Lines_k[l],a,t]) for a in model.Phases_abc)
        model.Iflow_re = Expression(model.Lines,model.Phases_abc,model.time, rule=I_flow_re_rule) 
        def I_flow_im_rule(model, l, s, t): # Imaginary line flows
            return sum(model.Bus_B[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vre[model.Lines_i[l],a,t]-model.Vre[model.Lines_k[l],a,t]) + model.Bus_G[model.Lines_k[l],model.Lines_i[l],s,a]*(model.Vim[model.Lines_i[l],a,t]-model.Vim[model.Lines_k[l],a,t]) for a in model.Phases_abc)
        model.Iflow_im = Expression(model.Lines,model.Phases_abc,model.time, rule=I_flow_im_rule) 
    # PQ line flows
    if 1==1:
        def P_flow_sending_rule(model, l, s, t): # P sending flow
            return model.Vre[model.Lines_k[l],s,t] * model.Iflow_re[l,s,t] + model.Vim[model.Lines_k[l],s,t] * model.Iflow_im[l,s,t]
        model.P_flow_sending = Expression(model.Lines,model.Phases_abc,model.time, rule=P_flow_sending_rule)    
        def P_flow_receiving_rule(model, l, s, t): # P receiving flow
            return -1 *(model.Vre[model.Lines_i[l],s,t] * model.Iflow_re[l,s,t] + model.Vim[model.Lines_i[l],s,t] * model.Iflow_im[l,s,t])  
        model.P_flow_receiving = Expression(model.Lines,model.Phases_abc,model.time, rule=P_flow_receiving_rule)
        def Q_flow_sending_rule(model, l, s, t): # Q sending flow
            return model.Vim[model.Lines_k[l],s,t] * model.Iflow_re[l,s,t] - model.Vre[model.Lines_k[l],s,t] * model.Iflow_im[l,s,t]
        model.Q_flow_sending = Expression(model.Lines,model.Phases_abc,model.time, rule=Q_flow_sending_rule)    
        def Q_flow_receiving_rule(model, l, s, t): # Q receiving flow
            return -1 *(model.Vim[model.Lines_i[l],s,t] * model.Iflow_re[l,s,t] - model.Vre[model.Lines_i[l],s,t] * model.Iflow_im[l,s,t])  
        model.Q_flow_receiving = Expression(model.Lines,model.Phases_abc,model.time, rule=Q_flow_receiving_rule)
    # Losses
    if 1==1:
        # Losses for this time step per line and phase
        def P_losses_rule(model, l, s, t): # Active power losses Watts!
            return model.P_flow_sending[l,s,t] + model.P_flow_receiving[l,s,t]
        model.P_losses = Expression(model.Lines,model.Phases_abc,model.time, rule=P_losses_rule)   
        def Q_losses_rule(model, l, s, t): # Reactive power losses Vars!
            return model.Q_flow_sending[l,s,t] + model.Q_flow_receiving[l,s,t]
        model.Q_losses = Expression(model.Lines,model.Phases_abc,model.time, rule=Q_losses_rule)
        # Total losses for this time step
        def P_losses_total_rule(model, t): # Total active power losses Watts!
            return sum(sum(model.P_losses[l,s,t] for s in model.Phases_abc) for l in model.Lines)
        model.P_losses_total = Expression(model.time, rule=P_losses_total_rule)
        def Q_losses_total_rule(model, t): # Total active power losses Watts!
            return sum(sum(model.Q_losses[l,s,t] for s in model.Phases_abc) for l in model.Lines)
        model.Q_losses_total = Expression(model.time, rule=Q_losses_total_rule)
        # Total Energy losses
        def P_losses_energy_rule(model): # P energy losses
            return sum(model.P_losses_total[t] for t in model.time)*(model.Timestep/60)
        model.P_losses_energy = Expression(rule=P_losses_energy_rule)
        def Q_losses_energy_rule(model): # Q energy losses
            return sum(model.Q_losses_total[t] for t in model.time)*(model.Timestep/60)
        model.Q_losses_energy = Expression(rule=Q_losses_energy_rule)       

## Constraints ##
if 1==1:
    # Power flow constraints
    if 1==1:
        # Current mismatch
        def I_delta_re_rule(model, k, s, t): # Real part
            if k=='slack':
                return Constraint.Skip
            else:
                return model.Isp_re[k,s,t] - model.Icalc_re[k,s,t] == 0
        model.Idelta_re_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=I_delta_re_rule)
        def I_delta_im_rule(model, k, s, t): # Imaginary part
            if k=='slack':
                return Constraint.Skip
            else:
                return model.Isp_im[k,s,t] - model.Icalc_im[k,s,t] == 0
        model.Idelta_im_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=I_delta_im_rule)
        # Slack constraint
        def Slack_re_rule(model, k, s, t): # Real part
            if k=='slack':
                return model.Vre[k,s,t]==model.Vinitre_pu[k,s,t]*model.Vinit[k,s,t]
            else:
                return Constraint.Skip
        def Slack_im_rule(model, k, s, t): # Imaginary part
            if k=='slack':
                return model.Vim[k,s,t]==model.Vinitim_pu[k,s,t]*model.Vinit[k,s,t]
            else:
                return Constraint.Skip
        model.Slack_re_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=Slack_re_rule) # Real part
        model.Slack_im_con = Constraint(model.Buses,model.Phases_abc,model.time, rule=Slack_im_rule) # Imaginary part
    

## Objective function ##
if 1==1:
    # Dummy objective function
    if 1==1:
        model.obj = Objective(expr=model.Dummy)