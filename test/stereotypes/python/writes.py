# Writes: an assignment, an augmented assignment, del, and the targets of a tuple, a loop, or a with clause

class Buffer:
    def __init__(self):
        self.start = 0
        self.end = 0
        self.data = {}
        self.cursor = None

    # Each name of a tuple of targets is written
    # @stereotype command
    def reset(self):
        self.start, self.end = 0, 0

    # @stereotype command
    def swap(self):
        [self.start, self.end] = [self.end, self.start]

    # Each target of a chain of assignments is written
    # @stereotype command
    def rewind(self):
        self.start = self.end = 0

    # @stereotype set
    def forget(self):
        del self.cursor

    # A write through an index changes the data member
    # @stereotype set
    def put(self, key, value):
        self.data[key] = value

    # @stereotype set
    def drop(self, key):
        del self.data[key]

    # The target of a loop is written
    # @stereotype set
    def seek_last(self, positions):
        for self.cursor in positions:
            pass

    # The target of a with clause is written
    # @stereotype set
    def open(self, path):
        with open(path) as self.cursor:
            pass

    # An annotated assignment writes its target
    # @stereotype set
    def close(self):
        self.cursor: object = None

    # A comparison writes nothing
    # @stereotype predicate
    def is_empty(self):
        return self.start == self.end

    # A keyword argument is no write
    # @stereotype wrapper
    def dump(self):
        emit(self.data, end=self.end)

    # A write through a member of a parameter changes the object of the caller and no data member
    # @stereotype controller incidental
    def stamp(self, entry):
        entry.owner = "buffer"

    # A write of a local changes nothing but the local
    # @stereotype incidental
    def scratch(self, entry):
        entry = None
