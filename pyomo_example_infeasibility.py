# Simple linear programming problem with method to check infeasibility

import shutil
import os
import sys
import logging

from pyomo.util.infeasible import log_infeasible_constraints
from pyomo.opt import SolverFactory, SolverStatus, TerminationCondition
import pyomo.environ as pyo


def check_pyomo_environment():
    """
    """
    if not shutil.which("pyomo"):
        os.system('pip install -q pyomo')
        assert (shutil.which("pyomo"))

    if not (shutil.which("cbc") or os.path.isfile("cbc")):
        if "google.colab" in sys.modules:
            os.system('apt-get install -y -qq coinor-cbc')
        else:
            try:
                os.system('conda install -c conda-forge coincbc')
            except:
                pass

    assert (shutil.which("cbc") or os.path.isfile("cbc"))


def initialize_logger(name):
    """
    Prepare logging
    """
    logger = logging.getLogger('{}'.format(name))
    logger.setLevel(logging.DEBUG)
    fh = logging.FileHandler('{}.log'.format(name))
    fh.setLevel(logging.DEBUG)
    logger.addHandler(fh)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    logger.addHandler(fh)
    logging.getLogger().addHandler(logging.StreamHandler())  # to display in console message
    # logger.debug('mensaje debug')
    # logger.info('mensaje info')
    # logger.warning('mensaje warning')
    # logger.error('mensaje error')
    # logger.critical('mensaje critical')
    return logger


def define_model():
    """
    Define linear optimiztion problem
    """
    model = pyo.ConcreteModel()
    # for access to dual solution for constraints
    model.dual = pyo.Suffix(direction=pyo.Suffix.IMPORT)
    # Sets
    model.I = {1, 2, 3}
    # Variables
    model.x = pyo.Var(model.I, initialize=0)
    # Parameters
    model.M = pyo.Param(initialize=10 ** 2)

    def obj_rule(model):
        return sum(model.x[i] * 1 for i in model.I)

    model.z = pyo.Objective(rule=obj_rule, sense=pyo.maximize)

    # m.cons1 = ConstraintList()
    # for i in m.LE:
    #    m.cons1.add(10**2 * m.x[i] >= m.M)
    #    m.cons1.add(10**2 * m.x[i] <= -3)

    def constraint_rule_1(model, i):
        return model.x[i] >= model.M

    model.constraint_1 = pyo.Constraint(model.I, rule=constraint_rule_1)

    def constraint_rule_2(model, i):
        return model.x[i] <= -model.M

    model.constraint_2 = pyo.Constraint(model.I, rule=constraint_rule_2)

    return model


def has_solution(results):
    """
    Check the solution
    """
    status = results.solver.status
    condition = results.solver.termination_condition

    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    feasible = condition == pyo.TerminationCondition.optimal or condition == pyo.TerminationCondition.feasible
    solution = (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit)

    if not solution:
        logger.error(results)
        logger.error('Something is wrong -> status {s} and condition {c}'.format(s=status, c=condition))
        if condition == pyo.TerminationCondition.infeasible:
            log_infeasible_constraints(model,
                                       logger=logger,
                                       log_variables=True, log_expression=True)

    if (results.solver.status == SolverStatus.ok) and (
            results.solver.termination_condition == TerminationCondition.optimal):
        print(':::Feasible solution:::')

        print('--- Problem Results ---')
        print(results.Problem())
        print('--- Solver Results ---')
        print(results.Solver())

    elif results.solver.termination_condition == TerminationCondition.infeasible:
        print('(!) Infeasible solution')
    else:
        print('(!)')
        print('Status:', results.solver.status)
        # Something else is wrong
        print('termination condition:', results.solver.termination_condition)


def get_dual_variables():
    """
    """
    print("Constraint  value  lslack  uslack    dual")
    for constraint in [model.z, model.constraint_1, model.constraint_2]:
        print(constraint, str.format(constraint(),
                                     constraint.lslack(),
                                     constraint.uslack(),
                                     model.dual[constraint]))


def get_slacks():
    """
    """
    for c in model.component_objects(ctype=pyo.Constraint):
        print(c)
        if c.slack() < 0:  # constraint is not met
            print(f'Constraint {c.name} is not satisfied')
            c.display()  # show the evaluation of c
            c.pprint()  # show the construction of c
            print()


if __name__ == '__main__':
    check_pyomo_environment()
    logger = initialize_logger('pyomo_example_infeasibility')
    model = define_model()
    results = SolverFactory('glpk').solve(model, tee=True)
    has_solution(results)
    # get_dual_variables()
    # get_slacks()
