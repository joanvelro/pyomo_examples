import random
from deap import base, creator, tools, algorithms

# Problem definition
machines = [0, 1, 2]  # List of machines
jobs = [
    [(0, 3), (1, 2), (2, 2)],  # Job 1: (machine, time)
    [(0, 2), (2, 1), (1, 4)],  # Job 2: (machine, time)
    [(1, 4), (2, 3)],  # Job 3: (machine, time)
]

num_jobs = len(jobs)
num_machines = len(machines)

# Genetic Algorithm setup
creator.create("FitnessMin", base.Fitness, weights=(-1.0,))
creator.create("Individual", list, fitness=creator.FitnessMin)

toolbox = base.Toolbox()

# Define an individual as a permutation of job indices
toolbox.register("indices", random.sample, range(num_jobs), num_jobs)
toolbox.register("individual", tools.initIterate, creator.Individual, toolbox.indices)
toolbox.register("population", tools.initRepeat, list, toolbox.individual)


# Evaluation function to calculate makespan
def evaluate(individual):
    machine_times = [0] * num_machines  # Track the end time of each machine
    job_times = [0] * num_jobs  # Track the end time of each job

    for job_index in individual:
        job = jobs[job_index]
        for machine, time in job:
            start_time = max(machine_times[machine], job_times[job_index])
            machine_times[machine] = start_time + time
            job_times[job_index] = machine_times[machine]

    return max(machine_times),


toolbox.register("mate", tools.cxTwoPoint)
toolbox.register("mutate", tools.mutShuffleIndexes, indpb=0.05)
toolbox.register("select", tools.selTournament, tournsize=3)
toolbox.register("evaluate", evaluate)


def main():
    pop = toolbox.population(n=50)
    hof = tools.HallOfFame(1)
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("min", min)
    stats.register("avg", lambda pop: sum(ind.fitness.values[0] for ind in pop) / len(pop))

    algorithms.eaSimple(pop, toolbox, cxpb=0.7, mutpb=0.2, ngen=40, stats=stats, halloffame=hof, verbose=True)

    return pop, stats, hof


if __name__ == "__main__":
    pop, stats, hof = main()
    best_individual = hof[0]
    print(f"Best individual is: {best_individual}")
    print(f"With makespan: {evaluate(best_individual)[0]}")