import logging
import time
import numpy as np

from data import hydrolubes
from model_data import ModelData

logger = logging.getLogger(__name__)


class ModelDataFactory(object):
    """
    TBD
    """

    def __init__(self) -> None:
        self.data = ModelData()

    def create(self, STN: dict = None):

        # Create data instance
        try:
            start = time.time()
            logger.info('Building data model...')

            if STN is None:
                STN = hydrolubes()

            STATES = STN['STATES']
            ST_ARCS = STN['ST_ARCS']
            TS_ARCS = STN['TS_ARCS']
            UNIT_TASKS = STN['UNIT_TASKS']
            TIME = STN['TIME']
            H = max(TIME)

            # set of tasks
            TASKS = set([i for (j, i) in UNIT_TASKS])

            # S[i] input set of states which feed task i
            S = {i: set() for i in TASKS}
            for (s, i) in ST_ARCS:
                S[i].add(s)

            # S_[i] output set of states fed by task i
            S_ = {i: set() for i in TASKS}
            for (i, s) in TS_ARCS:
                S_[i].add(s)

            # rho[(i,s)] input fraction of task i from state s
            rho = {(i, s): ST_ARCS[(s, i)]['rho'] for (s, i) in ST_ARCS}

            # rho_[(i,s)] output fraction of task i to state s
            rho_ = {(i, s): TS_ARCS[(i, s)]['rho'] for (i, s) in TS_ARCS}

            # P[(i,s)] time for task i output to state s
            P = {(i, s): TS_ARCS[(i, s)]['dur'] for (i, s) in TS_ARCS}

            # p[i] completion time for task i
            p = {i: max([P[(i, s)] for s in S_[i]]) for i in TASKS}

            # K[i] set of units capable of task i
            K = {i: set() for i in TASKS}
            for (j, i) in UNIT_TASKS:
                K[i].add(j)

            # characterization of states

            # T[s] set of tasks receiving material from state s
            T = {s: set() for s in STATES}
            for (s, i) in ST_ARCS:
                T[s].add(i)

            # set of tasks producing material for state s
            T_ = {s: set() for s in STATES}
            for (i, s) in TS_ARCS:
                T_[s].add(i)

            # C[s] storage capacity for state s
            C = {s: STATES[s]['capacity'] for s in STATES}

            # characterization of units

            UNITS = set([j for (j, i) in UNIT_TASKS])

            # I[j] set of tasks performed with unit j
            I = {j: set() for j in UNITS}
            for (j, i) in UNIT_TASKS:
                I[j].add(i)

            # Bmax[(i,j)] maximum capacity of unit j for task i
            Bmax = {(i, j): UNIT_TASKS[(j, i)]['Bmax'] for (j, i) in UNIT_TASKS}

            # Bmin[(i,j)] minimum capacity of unit j for task i
            Bmin = {(i, j): UNIT_TASKS[(j, i)]['Bmin'] for (j, i) in UNIT_TASKS}

            TIME = np.array(TIME)

            model_data_dict = {'STATES': STATES,
                               'ST_ARC': ST_ARCS,
                               'TS_ARCS': TS_ARCS,
                               'UNIT_TASKS': UNIT_TASKS,
                               'TIME': TIME,
                               'H': H,
                               'TASKS': TASKS,
                               'S': S,
                               'S_': S_,
                               'rho': rho,
                               'rho_': rho_,
                               'P': P,
                               'p': p,
                               'K': K,
                               'T': T,
                               'T_': T_,
                               'Bmin': Bmin,
                               'Bmax': Bmax,
                               'I': I,
                               'UNITS': UNITS,
                               'C': C}

            self.data.set_data_dict(data_dict=model_data_dict)
            end = time.time()
            logger.info(f'done! It took {round(end - start, 3)} seconds\n')

        except Exception as err:
            raise Exception('Unexpected error building data model') from err

        return self.data
