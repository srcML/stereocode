# Accessors (Method-Stereotypes wiki, Structural Accessor): get, predicate, property, and void-accessor

class Thermometer:
    # @stereotype constructor
    def __init__(self, celsius: float):
        self.celsius = celsius
        self.calibrated = False

    # @stereotype get
    def get_celsius(self) -> float:
        return self.celsius

    # @stereotype get
    def is_calibrated(self) -> bool:
        return self.calibrated

    # @stereotype predicate
    def is_freezing(self) -> bool:
        return self.celsius <= 0.0

    # @stereotype property
    def get_fahrenheit(self) -> float:
        return self.celsius * 9.0 / 5.0 + 32.0

    # A write through an index of a parameter changes the list of the caller
    # @stereotype void-accessor
    def read_celsius(self, values: list[float]) -> None:
        values[0] = self.celsius


# The same accessors without annotations, where the returned values give the return type
class Barometer:
    # @stereotype constructor
    def __init__(self, pressure):
        self.pressure = pressure
        self.ready = False

    # @stereotype get
    def get_pressure(self):
        return self.pressure

    # A comparison is Boolean by its form
    # @stereotype predicate
    def is_low(self):
        return self.pressure < 1000

    # @stereotype predicate
    def is_usable(self):
        return not self.ready or self.pressure == 0

    # A name is not Boolean by its form, so the conjunction of two names is no Boolean
    # @stereotype property
    def reading(self):
        return self.ready and self.pressure

    # @stereotype property
    def in_bar(self):
        return self.pressure / 1000

    # A parameter without an annotation shares its object with the caller
    # @stereotype void-accessor
    def read_pressure(self, values):
        values[0] = self.pressure
