# Free functions (Free-Function-Stereotypes wiki): the functions of a module, static methods, and class methods
# predicate, property, global-command, command, factory, main, literal, wrapper, empty, and unclassified

calls = 0


# @stereotype predicate
def is_even(value: int) -> bool:
    return value % 2 == 0


# @stereotype property
def square(value: int) -> int:
    return value * value


# A name of a global statement is the global of the module
# @stereotype global-command
def add_calls(count: int) -> None:
    global calls
    calls += count


# A write through an index of a parameter changes the list of the caller
# @stereotype command
def fill(values: list[int], value: int) -> None:
    values[0] = value


# @stereotype property factory
def make_widget(style: int) -> Widget:
    widget = Widget(style)
    return widget


# @stereotype literal
def answer() -> int:
    return 42


# @stereotype wrapper
def report(value: int) -> None:
    journal.write(value)


# @stereotype empty
def noop(value: int) -> None:
    pass


# print is an ignored call, so a function that only reads its parameter fits no rule
# @stereotype unclassified
def show(value: int) -> None:
    print(value)


# A parameter that the function calls is read, so the function is no literal
# @stereotype wrapper
def run(callback) -> None:
    callback()


# A function of the archive is called, never created, also where its name has the form of the name of a class
# @stereotype property
def Twice(value):
    return 2 * value


# @stereotype property wrapper
def Quadruple(value):
    return Twice(Twice(value))


# @stereotype wrapper
def Report(value) -> None:
    journal.Twice(value)


# A call of a parameter that creates an object reads the parameter
# @stereotype property factory
def build(Shape):
    return Shape()


user32 = ctypes.windll.user32


# A name that is called on a global of the file names a method of the object that the global holds, which the call may change
# @stereotype predicate global-command wrapper
def is_key_down(key):
    return (user32.GetKeyState(key) & 0x8000) != 0


try:
    import gzip
except ImportError:
    gzip = None


# A name that the file imports names a module, also where the file binds the name again when the import fails,
# so a call of a function of the module changes no global
# @stereotype property wrapper
def unzip(data):
    return gzip.decompress(data)


# A call of a method of the built-in types that tests its object is Boolean by its form
# @stereotype predicate wrapper
def is_private(name):
    return name.startswith("_")


# A call on an element of a parameter may change the object of the caller, as a call on the parameter itself may
# @stereotype command
def close_first(doors):
    doors[0].close()


# range stands for the header of a counting loop, so its call is ignored, as the call of len is
# @stereotype property
def total(values):
    result = 0
    for index in range(len(values)):
        result += values[index]
    return result


# @stereotype main
def main() -> None:
    report(1)


class Shape:
    count = 0

    def __init__(self, sides: int):
        self.sides = sides

    # A static method is a free function
    # @stereotype property
    @staticmethod
    def perimeter(side: float, sides: int) -> float:
        return side * sides

    # The first parameter of a class method names the class, so its call creates an object
    # @stereotype factory literal
    @classmethod
    def triangle(cls) -> "Shape":
        return cls(3)

    # @stereotype property factory
    @classmethod
    def polygon(cls, sides: int):
        return cls(sides)

    # A name reached through the class is a variable of the class, which is a global
    # @stereotype global-command
    @classmethod
    def register(cls) -> None:
        cls.count += 1

    # @stereotype global-command
    @staticmethod
    def forget() -> None:
        Shape.count = 0

    # A class method declares a variable of the class by a write through the class
    # @stereotype global-command
    @classmethod
    def mark(cls) -> None:
        cls.marked = True

    # @stereotype unclassified
    @classmethod
    def is_marked(cls) -> bool:
        return cls.marked

    # A variable of the class may have the name of a parameter
    # @stereotype global-command
    @classmethod
    def resize(cls, count: int) -> None:
        cls.count = count

    # __init_subclass__ and __class_getitem__ are class methods without a decorator
    # @stereotype global-command
    def __init_subclass__(cls) -> None:
        cls.count += 1
