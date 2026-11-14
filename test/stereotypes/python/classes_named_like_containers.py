# A class of the archive with the name of a container of Python, as Counter, names its own type, unlike collections.Counter


class Counter:
    def __init__(self):
        self.total = 0

    # @stereotype get
    def value(self) -> int:
        return self.total


class Tally:
    def __init__(self):
        self.counter = Counter()
        self.limit = 0

    # @stereotype property collaborator @defect the class counts as the container collections.Counter
    def current(self) -> Counter:
        return self.counter if self.limit > 0 else None

    # @stereotype set collaborator @defect the class counts as the container collections.Counter
    def take(self, other: Counter):
        self.limit = other.total
