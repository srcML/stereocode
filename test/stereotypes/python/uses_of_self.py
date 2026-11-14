# The object: the first parameter of a method names it, under any name (Method-Stereotypes wiki, get, set, and command)

class Builder:
    def __init__(self):
        self.width = 0
        self.height = 0

    # @stereotype get
    def itself(self):
        return self

    # A set or a command may return the object, as a method whose calls chain does
    # @stereotype set
    def with_width(self, width):
        self.width = width
        return self

    # @stereotype command
    def with_size(self, width, height):
        self.width = width
        self.height = height
        return self

    # The first parameter names the object under any name
    # @stereotype get
    def get_width(this):
        return this.width

    # @stereotype set
    def set_height(me, height):
        me.height = height

    # The object is no parameter, so its use makes no collaborator and no void-accessor
    # @stereotype property
    def area(self: "Builder") -> int:
        return self.width * self.height

    # A name that only starts with the name of the object is another variable
    # @stereotype incidental
    def selfish(self, selfie):
        return selfie.width

    # The object passed to a function is a use of the object
    # @stereotype wrapper
    def register(self):
        registry.add(self)

    # A rebinding of the name of the object changes no data member
    # @stereotype unclassified
    def detach(self, other):
        self = other
