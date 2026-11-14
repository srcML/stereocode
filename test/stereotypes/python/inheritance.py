# Inheritance: a class inherits the data members and the methods of each parent of the archive

class Vehicle:
    def __init__(self):
        self.wheels = 4
        self.speed = 0

    # @stereotype set
    def set_speed(self, speed):
        self.speed = speed

    # @stereotype get
    def get_wheels(self):
        return self.wheels

    # @stereotype get
    def describe(self):
        return self.speed


class Truck(Vehicle):
    def __init__(self):
        super().__init__()
        self.load = 0

    # A data member of a parent is a data member of the object
    # @stereotype property
    def axle_load(self):
        return self.load / self.wheels

    # A method of a parent is called through the object
    # @stereotype command
    def halt(self):
        self.set_speed(0)

    # super() reaches the methods of a parent, as super does in Java
    # @stereotype command
    def brake(self):
        super().set_speed(0)

    # A call to an accessor of a parent reads a data member
    # @stereotype predicate
    def is_heavy(self):
        return self.load > 100 * self.get_wheels()

    # @stereotype property
    def describe(self):
        return super().describe() + self.load


# A parent is known by the last part of its name, and a keyword of the list of parents is no parent
class Trailer(fleet.Truck, metaclass=Registered):
    # @stereotype predicate
    def is_empty(self):
        return self.load == 0 and self.get_wheels() > 0

    # @stereotype command
    def park(self):
        self.halt()


# A parent outside the archive may define the method that a call names, which may change the object
class Dialog(Window):
    def __init__(self):
        self.title = ""

    # @stereotype command
    def open(self):
        self.show()

    # @stereotype get
    def get_title(self):
        return self.title

    # A method of the class takes every call of its name through the object, since a Python class has one method per name
    # @stereotype property
    def depth(self, level):
        return level if level > len(self.title) else self.depth(level + 1)

    # @stereotype property
    def joined(self, first, second):
        return self.title + first + second

    # The call with unpacked arguments names the method of the class as well
    # @stereotype property
    def joined_all(self, parts):
        return self.joined(*parts)


# A method called under the name of the class or of a parent, with the object as its first argument, is called through the object,
# as a method after super() is
class Van(Vehicle):
    # @stereotype command
    def stop(self):
        Vehicle.set_speed(self, 0)

    # @stereotype property
    def twice(self):
        return Vehicle.describe(self) * 2

    # A method of another class that takes the object is no method of the object
    # @stereotype wrapper
    def log(self):
        Registry.note(self)


# A query method of a parent outside the archive reads the object
class Table(dict):
    # @stereotype command
    def put(self, key, value):
        dict.__setitem__(self, key, value)

    # @stereotype property
    def fetch(self, key):
        return dict.get(self, key) or 0

    # @stereotype property
    def fetch_twice(self, key):
        return self.get(key) * 2


# Every object inherits the accessors of the class object
class Frozen:
    def __init__(self, x):
        object.__setattr__(self, "x", x)

    # @stereotype command
    def move(self, x):
        object.__setattr__(self, "x", x)

    # @stereotype property
    def label(self):
        return super().__repr__() + "!"

    # @stereotype property
    def raw(self, name):
        return object.__getattribute__(self, name) or 0
