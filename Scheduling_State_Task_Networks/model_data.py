class ModelData(object):

    def __init__(self) -> None:
        self.__data_dict = {}

    def set_data_dict(self, data_dict):
        self.__data_dict = data_dict

    def get_data_dict(self):
        return self.__data_dict
