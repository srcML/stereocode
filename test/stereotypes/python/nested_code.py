# Nested code: a function in a function gets no label, and its code is code of the function around it, as the code of a lambda is,
# while a class in a class is a type of its own

class Sorter:
    def __init__(self):
        self.key = 0
        self.items = []

    # The body of a nested function is code of the method, while its return is its own
    # @stereotype property
    def make_reader(self):
        def read():
            return self.key
        return read

    # @stereotype property non-void-command
    def make_writer(self):
        self.key = 1

        def write(value):
            self.items.append(value)
        return write

    # A lambda belongs to the method around it, and its parameter is a local
    # @stereotype property
    def by_key(self):
        return lambda item: item + self.key

    # @stereotype command
    def order(self):
        self.items.sort(key=lambda item: item % 10)

    # A call of a nested function runs code of the method itself, so it is no call
    # @stereotype set
    def bump(self):
        def step():
            self.key += 1
        step()

    # A parameter of a nested function is a local of the method
    # @stereotype incidental
    def seven(self):
        def add(left, right):
            return left + right
        return add(3, 4)


# @stereotype data-provider data-class small-class
class Outer:
    def __init__(self):
        self.size = 0

    # @stereotype data-provider data-class small-class
    class Inner:
        def __init__(self):
            self.depth = 0

        # @stereotype get
        def get_depth(self):
            return self.depth

    # @stereotype get
    def get_size(self):
        return self.size


# A nested function is no free function, so only the function around it gets a label
# @stereotype property
def scale_all(values, factor):
    def scale(value):
        return value * factor
    return [scale(value) for value in values]


# The call of the parameter in the nested function is a call of the function around it, which reads the parameter
# @stereotype property wrapper
def traced(function):
    def call(*arguments):
        return function(*arguments)
    return call
