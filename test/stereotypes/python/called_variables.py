# A call of a variable runs the function that the variable holds, so the call reads the variable and stays a call of a function
# (Method-Stereotypes wiki, Calls; Free-Function-Stereotypes wiki, literal and wrapper)

import json


class Doubler:
    def __init__(self):
        self.factor = 2

    # @stereotype property
    def __call__(self, value):
        return self.factor * value


class Button:
    def __init__(self, on_fire):
        self.on_fire = on_fire
        self.doubler = Doubler()
        self.count = 0

    # A call of a data member reads the data member, so the method uses the state of the object
    # @stereotype wrapper
    def fire(self):
        self.on_fire()

    # A data member that holds an object of a class of the archive calls the method __call__ of that class
    # @stereotype property collaborator
    def scaled(self):
        return self.doubler(self.count) + 1

    # @stereotype wrapper
    def each(self, visit):
        visit(self.count)

    # @stereotype get
    def handler(self):
        return self.on_fire

    # @stereotype wrapper
    def start(self):
        held = self.handler()
        held()

    # A local that a lambda initializes holds code of the method itself, so its call is no call
    # @stereotype property
    def twice(self):
        doubled = lambda value: 2 * value
        return doubled(self.count)


# A call of a parameter reads the parameter, so the function is no literal, and changes no object of the caller
# @stereotype wrapper
def run(callback):
    callback()


# @stereotype property
def compute(value):
    square = lambda number: number * number
    return square(value) + 1


parse = json.loads


# @stereotype property wrapper
def load(text):
    return parse(text)
