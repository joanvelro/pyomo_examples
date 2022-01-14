from abc import ABC, abstractmethod

# abstract Class
class person(ABC):

    @abstractmethod
    def name(self):
        pass

    @abstractmethod
    def surname(self):
        pass

    @abstractmethod
    def salary(self):
        pass

class data_scientist(person):
    def __init__(self):
        self.me = "me"

    def name(self):
        print('Data Scientist')

    def surname(self):
        pass

    def salary(self):
        self.salary = 55000
        print(self.salary)

    def data_analysis(self):
        print('data_analysis')


class data_engineer(person):
    def __init__(self):
        self.me = "me"

    def name(self):
        print('Data Engineer')

    def surname(self):
        pass

    def salary(self):
        self.salary = 45000
        print(self.salary)

    def concrete_method(self):
        print('this method is concrete from Concrete_Class')
    
    def data_engineering(self):
        print('data_engineering')

# The following would raise the TypeError complaining abstracteMethod is not impliemented
ds = data_scientist()  
de = data_engineer()  