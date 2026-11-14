# Return types: the annotation, or the returned values of a function without one

class Sensor:
    def __init__(self):
        self.level = 0
        self.history = []
        self.label = ""

    # A method that returns no value returns None
    # @stereotype command
    def clear(self):
        self.level = 0
        self.history.clear()
        return

    # @stereotype command
    def wipe(self):
        self.level = 0
        self.history.clear()
        return None

    # A returned value that is no Boolean makes the method return a value
    # @stereotype property non-void-command
    def drop(self):
        self.level = 0
        self.history.clear()
        return 0

    # A comparison, a negation, a Boolean literal, and a test of membership or of a type are Boolean by their form
    # @stereotype predicate
    def is_high(self):
        return self.level > 10

    # @stereotype predicate
    def is_idle(self):
        if self.level:
            return False
        return not self.history

    # @stereotype predicate
    def is_known(self):
        return self.level in self.history

    # @stereotype predicate
    def is_number(self):
        return isinstance(self.level, int)

    # A call of a method of the built-in types that tests its object, as startswith of a string, is Boolean by its form
    # @stereotype predicate
    def is_labeled(self):
        return self.label.startswith("level")

    # @stereotype predicate
    def is_shouted(self):
        return self.label[:1].isupper()

    # A conditional expression is Boolean when its two results are
    # @stereotype predicate
    def is_safe(self):
        return True if self.level < 5 else self.level == 7

    # @stereotype property
    def pick(self):
        return True if self.level < 5 else self.level

    # A name is not Boolean by its form
    # @stereotype property
    def state(self):
        return self.level or self.history

    # A method that returns a Boolean on one path and another value on the next returns no Boolean
    # @stereotype property
    def mixed(self):
        if self.level > 3:
            return True
        return self.level + 1

    # A yield passes its value to the caller, as a return does
    # @stereotype property
    def readings(self):
        for value in self.history:
            yield value * 2

    # @stereotype get
    def each(self):
        yield from self.history

    # An annotation holds over the form of the returned values
    # @stereotype predicate
    def ready(self) -> bool:
        return self.level and self.history

    # @stereotype property
    def flag(self) -> int:
        return self.level > 0

    # A returned object that the method creates gives the return type its class
    # @stereotype property factory collaborator
    def snapshot(self):
        return Reading(self.level)

    # An asynchronous method returns the type of its annotation
    # @stereotype predicate wrapper
    async def poll(self) -> bool:
        return await fetch() > self.level


# A container of Boolean values returns the container, which is no Boolean, while an optional Boolean is one
class Switches:
    def __init__(self):
        self.states = []
        self.count = 0

    # @stereotype property
    def active(self) -> list[bool]:
        return self.states if self.count > 0 else []

    # @stereotype predicate
    def ready(self) -> Optional[bool]:
        return self.count > 0 if self.states else None
