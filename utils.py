from typing import Any

import pyomo.environ as pyo

from pyomo.util.infeasible import log_infeasible_constraints

import logging

logger = logging.getLogger(__name__)

logging.getLogger(__name__).addHandler(logging.NullHandler())


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

def add_stderr_logger(level: int = logging.INFO) -> logging.StreamHandler:
    """
    Helper for quickly adding a StreamHandler to the logger. Useful for
    debugging.
    Returns the handler after adding it.
    """
    # This method needs to be in this __init__.py to get the __name__ correct
    logger = logging.getLogger(__name__)
    handler = logging.StreamHandler()
    handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
    logger.addHandler(handler)
    logger.setLevel(level)
    logger.propagate = False
    return handler


def has_solution(results, model) -> bool:
    """
    Check if a feasible solution is found.
    """
    status = results.solver.status
    condition = results.solver.termination_condition

    infeasible = condition == pyo.TerminationCondition.infeasible
    unbounded = condition == pyo.TerminationCondition.unbounded
    unknown = condition == pyo.TerminationCondition.unknown
    infeasible_or_unbounded = condition == pyo.TerminationCondition.infeasibleOrUnbounded
    max_time_limit = condition == pyo.TerminationCondition.maxTimeLimit
    optimal = condition == pyo.TerminationCondition.optimal
    feasible = condition == pyo.TerminationCondition.feasible

    other_condition = condition == pyo.TerminationCondition.other
    solution = (status == pyo.SolverStatus.ok and optimal) or \
               (status == pyo.SolverStatus.ok and feasible) or \
               (status == pyo.SolverStatus.aborted and max_time_limit) or \
               (status == pyo.SolverStatus.ok and other_condition)  # GAP limit in SCIP

    if feasible:
        print('Feasible Solution founded !')
        print('\n')
    if optimal:
        print('Optimal Solution founded !')
        print('\n')

    if not solution:
        print(results)
        print(
            f"Something is wrong -> status {status} "
            f"and condition {condition}"
        )
        if infeasible or infeasible_or_unbounded or unbounded or unknown:
            # and (logger.getEffectiveLevel() <= logging.DEBUG):
            log_infeasible_constraints(
                model,
                logger=logger,
                log_expression=True,
                log_variables=True,
            )

    return solution


def generate_lp_file(model,
                     model_name,
                     variable_names: bool = True
                     ) -> None:

    lp_file_name = model_name + ".lp"
    print('\tGenerating LP model...{}'.format(lp_file_name))
    print("\tdone!")
    model.write(lp_file_name, io_options={"symbolic_solver_labels": variable_names})
