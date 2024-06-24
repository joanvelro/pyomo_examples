from engine import Engine
from data import hydrolubes
from factory import ModelDataFactory

if __name__ == "__main__":
    STN_instance = hydrolubes(H=24*7)
    data_instance = ModelDataFactory().create(STN=STN_instance)
    engine_instance = Engine(model_data=data_instance)
    engine_instance.execute(solver='cbc',
                            opt_params={"ratioGAP", 0.1})
    UnitAssignment = engine_instance.get_UnitAssignment()
    state_inventories = engine_instance.get_state_inventories()

    print(UnitAssignment)
    print(state_inventories)
    engine_instance.plot_results()
