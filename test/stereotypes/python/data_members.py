# Data members: a class declares none, so the body of the class and the writes of its methods tell them

from typing import ClassVar


# An annotated name of the class body is a data member, unless its annotation is ClassVar
class Pixel:
    x: int
    y: int = 0
    shades: ClassVar[int] = 256

    # @stereotype get
    def get_x(self):
        return self.x

    # @stereotype set
    def move_down(self):
        self.y += 1

    # A variable of the class is a global, so a use through the class reads no state of the object
    # @stereotype incidental
    def depth(self):
        return Pixel.shades

    # A name reached through the object is a data member, also where the body of the class gives its value
    # @stereotype get
    def shade_count(self):
        return self.shades


# A write of a variable of another class makes a collaborator, as a write of a static field of another class does
class Painter:
    def __init__(self):
        self.strokes = 0

    # @stereotype collaborator incidental
    def darken(self):
        Pixel.shades -= 1

    # A module is no class, so a write of a variable of a module makes no collaborator
    # @stereotype incidental
    def mute(self):
        logging.raiseExceptions = False

    # @stereotype collaborator incidental
    def hide(self):
        Canvas.visible = False


# The class of the object holds the variables of the class, which are no state of the object
class Tally:
    total = 0

    def __init__(self):
        self.count = 0

    # @stereotype incidental
    def bump_total(self):
        self.__class__.total += 1

    # @stereotype incidental
    def kind(self):
        return self.__class__.__name__

    # @stereotype set
    def bump(self):
        self.count += 1


# A parent outside the archive declares no name that the code reaches without the object
class Probe(Device):
    # @stereotype incidental
    def arm(self):
        Probe.armed = True

    # @stereotype set
    def start(self):
        self.armed = True


# The strings of __slots__ name data members
class Slotted:
    __slots__ = ("width", "height")

    # @stereotype property
    def area(self):
        return self.width * self.height


# A write through the object declares a data member in any method
class Cache:
    # @stereotype set
    def load(self, text):
        self.text = text

    # @stereotype get
    def text_of(self):
        return self.text

    # A name that no method writes is a data member as well, since only the object holds it
    # @stereotype get
    def source_of(self):
        return self.source


# The name of a method without a call is a method reference, which uses no state
class Button:
    def __init__(self):
        self.clicks = 0

    # @stereotype set
    def on_click(self):
        self.clicks += 1

    # @stereotype wrapper stateless
    def connect(self):
        subscribe(self.on_click)


# A data member may hold a function, whose call reads the data member and changes nothing of the object
class Task:
    def __init__(self, callback):
        self.callback = callback
        self.done = False

    # @stereotype set
    def finish(self):
        self.done = True
        self.callback()


# A method with the decorator of a property has stereotypes of its own, and its name is a data member for the other methods
class Circle:
    def __init__(self, radius: float):
        self._radius = radius

    # @stereotype get
    @property
    def radius(self) -> float:
        return self._radius

    # @stereotype set
    @radius.setter
    def radius(self, value: float) -> None:
        self._radius = value

    # @stereotype property
    @property
    def area(self) -> float:
        return 3.14159 * self._radius ** 2

    # @stereotype predicate
    def is_unit(self) -> bool:
        return self.radius == 1.0

    # @stereotype set
    def reset(self) -> None:
        self.radius = 1.0

    # @stereotype property
    def diameter(self):
        return 2 * self.radius

    # @stereotype property
    def ring(self, width: float) -> float:
        return self.area - 3.14159 * (self.radius - width) ** 2
