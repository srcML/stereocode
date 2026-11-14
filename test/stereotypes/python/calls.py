# Calls: a method is called through the object, and a call without an object calls a function

class Tank:
    def __init__(self):
        self.level = 0

    # @stereotype set
    def fill(self):
        self.level = 100

    # @stereotype get
    def get_level(self):
        return self.level


class Engine:
    def __init__(self):
        self.speed = 0
        self.parts = []
        self.fuel = Tank()

    # @stereotype set
    def set_speed(self, speed):
        self.speed = speed

    # @stereotype get
    def get_speed(self):
        return self.speed

    # A call to a mutator of the class may change the object
    # @stereotype command
    def stop(self):
        self.set_speed(0)

    # A call to an accessor of the class reads a data member
    # @stereotype predicate
    def is_moving(self):
        return self.get_speed() > 0

    # A call without an object calls a function, never a method of the class
    # @stereotype wrapper stateless
    def idle(self):
        stop()

    # A call on a data member may change the data member
    # @stereotype command
    def add_part(self, part):
        self.parts.append(part)

    # A query method only reads the data member that it is called on
    # @stereotype property
    def spare_parts(self):
        return self.parts.count(None)

    # A call on an element of a data member is a call on the data member, which a query method only reads
    # @stereotype command
    def tighten_first(self):
        self.parts[0].tighten()

    # @stereotype property
    def first_name(self):
        return self.parts[0].strip()

    # A call after the result of a call is a call on another object, also where a name comes between them
    # @stereotype controller stateless
    def vent_valve(self):
        find_hose().valve.open()

    # len stands for an operator, so its call is ignored
    # @stereotype property
    def part_count(self):
        return len(self.parts)

    # list stands for a conversion and range for the header of a counting loop, so their calls are ignored
    # @stereotype property
    def all_parts(self):
        return list(self.parts)

    # @stereotype property
    def part_weight(self):
        weight = 0
        for index in range(len(self.parts)):
            weight += self.parts[index]
        return weight

    # The class of a data member tells the method that a call on the data member calls
    # @stereotype command collaborator
    def refuel(self):
        self.fuel.fill()

    # @stereotype predicate collaborator
    def has_fuel(self):
        return self.fuel.get_level() > 0

    # A call on a parameter, on a local, or on the result of a call is a call on another object
    # @stereotype controller stateless
    def attach(self, hose):
        hose.connect()

    # @stereotype controller stateless
    def drain(self):
        hose = find_hose()
        hose.open()

    # @stereotype controller stateless
    def vent(self):
        find_hose().open()

    # A call of a function of a module is a call of a free function
    # @stereotype wrapper stateless
    def wait(self):
        time.sleep(1)

    # A call of a class creates an object, which is no call
    # @stereotype set collaborator
    def replace_tank(self):
        self.fuel = Tank()

    # raise holds a constructor call, as throw new does in the other languages
    # @stereotype stateless
    def fail(self):
        raise RuntimeError("no fuel")

    # print is an ignored call
    # @stereotype unclassified
    def show(self):
        print(self.speed)

    # sys.exit is ignored, also after a longer name, while exit of another object is a call
    # @stereotype unclassified
    def quit(self):
        print(self.speed)
        sys.exit(1)

    # @stereotype controller
    def leave(self, door):
        print(self.speed)
        door.exit()


class Ratio:
    def __init__(self, top, bottom):
        self.top = top
        self.bottom = bottom

    @staticmethod
    def gcd(a, b):
        return a if b == 0 else b

    @classmethod
    def whole(cls, value):
        return cls(value, 1)

    # A call of a static method or of a class method through the object is a call of a function, which changes no object
    # @stereotype property wrapper
    def reduced(self):
        return self.top / self.gcd(self.top, self.bottom)

    # @stereotype property wrapper
    def floor(self):
        return self.whole(self.top // self.bottom)

    # The name of a static method without a call is no data member
    # @stereotype wrapper stateless
    def publish(self):
        register(self.gcd)


class Mixed(Ratio):
    # @stereotype property wrapper
    def common(self):
        return self.gcd(self.top, self.bottom)
