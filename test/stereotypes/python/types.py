# Types: an annotation names the type of a parameter, a local, a data member, or a return value,
# and a name without an annotation has an unknown type, which counts as primitive

class Garage:
    def __init__(self, owner: Person, size: int):
        self.owner = owner
        self.size = size
        self.door = Door()
        self.cars: list[Car] = []
        self.plates: dict[str, int] = {}

    # A data member takes the type of the annotated parameter that the constructor assigns to it
    # @stereotype get collaborator
    def get_owner(self):
        return self.owner

    # @stereotype get
    def get_size(self):
        return self.size

    # A data member takes the class of the object that the constructor creates for it
    # @stereotype get collaborator
    def get_door(self):
        return self.door

    # The type of the elements of a container decides
    # @stereotype get collaborator
    def get_cars(self):
        return self.cars

    # @stereotype get
    def get_plates(self):
        return self.plates

    # A parameter without an annotation has an unknown type
    # @stereotype set
    def set_size(self, size):
        self.size = size

    # A quoted type is the type that it names
    # @stereotype set collaborator
    def lend(self, borrower: "Person") -> None:
        self.size = 0

    # Optional, Union, and the alternatives of | stand for the types that they hold
    # @stereotype set collaborator
    def reserve(self, guest: Optional[Person] = None) -> None:
        self.size = 1

    # @stereotype set collaborator
    def assign(self, guest: Person | None) -> None:
        self.size = 2

    # @stereotype set
    def mark(self, code: int | None) -> None:
        self.size = 3

    # A container of a primitive type is primitive
    # @stereotype set
    def resize(self, sizes: list[int]) -> None:
        self.size = sizes[0]

    # The type of a module is known by its last name
    # @stereotype set collaborator
    def insure(self, policy: insurance.Policy) -> None:
        self.size = 4

    # Self names the own type, which is no other class
    # @stereotype get
    def same(self) -> Self:
        return self

    # @stereotype predicate
    def is_larger(self, other: "Garage") -> bool:
        return self.size > other.size

    # The annotation of a local names its type
    # @stereotype collaborator incidental
    def probe(self) -> int:
        tool: Wrench = None
        return 0

    # A local takes the class of the object that the method creates for it
    # @stereotype collaborator stateless
    def inspect(self) -> None:
        tool = Wrench()

    # The return type of another class makes a collaborator, and an element of a data member is returned as the data member is
    # @stereotype get collaborator
    def first_car(self) -> Car:
        return self.cars[0]

    # @stereotype property collaborator
    def spare_car(self) -> Car:
        return self.cars[0] or self.cars[1]

    # The value of a Literal is no type
    # @stereotype set
    def paint(self, color: Literal["red", "blue"]) -> None:
        self.size = 5
