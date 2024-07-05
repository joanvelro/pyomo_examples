"""

Explanation

    Define the Trading Environment:
        A custom TradingEnv class extends gym.Env.
        The environment tracks the balance, positions, and net worth.
        Actions are 0 (hold), 1 (buy), and 2 (sell).

    Set Up the RL Model:
        We use Stable Baselines' A2C model.
        The model is trained on the trading environment.

    Integrate with Pyomo:
        A simple portfolio optimization problem is solved using Pyomo.
        The optimization function allocates funds based on initial prices.

This example provides a foundational structure for a trading model using RL and Pyomo. You can expand it by adding more sophisticated features and constraints.

"""

from pyomo.environ import ConcreteModel, Var, Objective, SolverFactory, Constraint

from stable_baselines3 import A2C
from stable_baselines3.common.env_checker import check_env

import gym
from gym import spaces
import numpy as np
import pandas as pd


class TradingEnv(gym.Env):
    def __init__(self, data):
        super(TradingEnv, self).__init__()
        self.data = data
        self.current_step = 0
        self.balance = 1000
        self.positions = 0
        self.net_worth = 1000
        self.max_steps = len(data)

        self.action_space = spaces.Discrete(3)  # 0: hold, 1: buy, 2: sell
        self.observation_space = spaces.Box(low=0, high=1, shape=(len(data.columns),), dtype=np.float32)

    def reset(self):
        self.current_step = 0
        self.balance = 1000
        self.positions = 0
        self.net_worth = 1000
        return self._next_observation()

    def _next_observation(self):
        obs = self.data.iloc[self.current_step].values
        return obs

    def step(self, action):
        current_price = self.data.iloc[self.current_step]['Close']

        if action == 1:  # buy
            self.positions += self.balance / current_price
            self.balance = 0
        elif action == 2:  # sell
            self.balance += self.positions * current_price
            self.positions = 0

        self.current_step += 1
        self.net_worth = self.balance + self.positions * current_price

        reward = self.net_worth - 1000
        done = self.current_step >= self.max_steps - 1
        obs = self._next_observation()

        return obs, reward, done, {}

    def render(self, mode='human', close=False):
        profit = self.net_worth - 1000
        print(f'Step: {self.current_step}')
        print(f'Balance: {self.balance}')
        print(f'Positions: {self.positions}')
        print(f'Net Worth: {self.net_worth}')
        print(f'Profit: {profit}')


def optimize_portfolio(prices):
    model = ConcreteModel()

    # Decision variables: allocation of funds to each stock
    model.x = Var(range(len(prices)), domain=NonNegativeReals)

    # Objective: maximize return
    model.obj = Objective(expr=sum(prices[i] * model.x[i] for i in range(len(prices))), sense=maximize)

    # Constraint: sum of allocations must be equal to available funds (e.g., $1000)
    model.budget = Constraint(expr=sum(model.x[i] for i in range(len(prices))) == 1000)

    solver = SolverFactory('glpk')
    solver.solve(model)

    return [model.x[i].value for i in range(len(prices))]


# Load your data
data = pd.read_csv('historical_stock_prices.csv')
env = TradingEnv(data)

# Check if the environment follows the Gym interface
check_env(env)

# Create the RL model
model = A2C('MlpPolicy', env, verbose=1)
model.learn(total_timesteps=10000)

# Save the model
model.save("trading_a2c")

# Reset the environment and get initial observation
obs = env.reset()

# Optimize portfolio based on initial prices
initial_prices = data.iloc[0].values
allocations = optimize_portfolio(initial_prices)
print(f'Optimized allocations: {allocations}')
