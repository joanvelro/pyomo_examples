import pyomo.environ as pyo
from pyomo.opt import SolverStatus, TerminationCondition
from pyomo.util.infeasible import log_infeasible_constraints
import logging

logger = logging.getLogger(__name__)

handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s %(message)s"))
logger.addHandler(handler)
logger.setLevel(logging.INFO)
logger.propagate = False

def build_model(graph):
    model = pyo.ConcreteModel()

    # Set of vertices
    model.vertex_set = pyo.Set(initialize=graph.vertices)

    # Set of colors
    model.colors_set = pyo.Set(initialize=['Color-{}'.format(i) for i in range(1, len(graph.vertices) + 1)])

    model.edges_set = pyo.Set(initialize=graph.edges)

    # Variable indicating the color assigned to each vertex
    model.color_var = pyo.Var(model.vertex_set, model.colors_set, within=pyo.Binary)

    # Objective function (not necessary for graph coloring, but can be added for extensions)
    model.obj = pyo.Objective(expr=1, sense=pyo.minimize)

    # Constraint: each vertex must be assigned exactly one color
    def one_color_rule(model, vertex):
        return sum(model.color_var[vertex, color] for color in model.colors_set) == 1

    model.one_color_constraint = pyo.Constraint(model.vertex_set, rule=one_color_rule)

    def one_vertex_color(model, color):
        return sum(model.color_var[vertex, color] for vertex in model.vertex_set) == 1

    model.one_vertex_color = pyo.Constraint(model.colors_set, rule=one_vertex_color)

    # Constraint: adjacent vertices must have different colors
    def different_colors_rule(model, vertex_u, vertex_v):
        return (sum(model.color_var[vertex_u, color] for color in model.colors_set) +
                sum(model.color_var[vertex_v, color] for color in model.colors_set) <= 1)

    #model.different_colors_constraint = pyo.Constraint(model.edges_set, rule=different_colors_rule)

    return model


# Sample graph represented by a list of vertices and edges_set
class BuildGraph:
    def __init__(self, vertices: list, edges: list[tuple[str, str]]):
        self.vertices = vertices
        self.edges = edges


# Example usage:
# Define a sample graph
vertices = ['A', 'B', 'C', 'D']
edges = [('A', 'B'), ('B', 'C'), ('C', 'D'), ('D', 'A'), ('B', 'D')]

graph = BuildGraph(vertices, edges)

# Create the Pyomo model
model = build_model(graph)

# Solve the model
solver = pyo.SolverFactory('cbc')  # Choose an appropriate solver
results = solver.solve(model, tee=True)


def has_solution(results, model):
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
        raise Exception('Error: Problem is infeasible or unbounded')

    return solution


solution = has_solution(results, model)

# Check the solution
if solution:
    print('--- Problem Results ---')
    print(results.Problem())
    print('--- Solver Results ---')
    print(results.Solver())

    # Display the results
    for v in model.vertex_set:
        for c in model.colors_set:
            if pyo.value(model.color_var[v, c]) == 1:
                print(f"Vertex {v} is assigned color {c}")

    model.pprint()
    print('hi')




