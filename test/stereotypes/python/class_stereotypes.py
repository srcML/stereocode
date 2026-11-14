# Class stereotypes (Class-Stereotypes wiki, rules for class stereotype identification)
# The method labels are listed to show the class signature; constructors do not count

# @stereotype entity boundary
class Measure:
    def __init__(self):
        self.value = 0
        self.changes = 0

    # @stereotype property collaborator
    def scaled(self, scale: Scale) -> int:
        return self.value * 2

    # @stereotype command collaborator
    def adjust(self, scale: Scale) -> None:
        self.value *= 2
        self.changes += 1

    # @stereotype get
    def get_value(self) -> int:
        return self.value


# @stereotype minimal-entity boundary
class Gauge:
    def __init__(self):
        self.level = 0
        self.limit = 0

    # @stereotype get collaborator
    def get_level(self, scale: Scale) -> int:
        return self.level

    # @stereotype set collaborator
    def set_level(self, amount: int, scale: Scale) -> None:
        self.level = amount

    # @stereotype command
    def fill(self) -> None:
        self.level = self.limit
        self.limit += 1


# @stereotype data-provider
class Circle:
    def __init__(self):
        self.radius = 0.0

    # @stereotype get
    def get_radius(self) -> float:
        return self.radius

    # @stereotype predicate
    def is_unit(self) -> bool:
        return self.radius == 1.0

    # @stereotype property
    def area(self) -> float:
        return 3.14159 * self.radius * self.radius

    # @stereotype set
    def set_radius(self, radius: float) -> None:
        self.radius = radius


# @stereotype minimal-entity commander
class Counter:
    def __init__(self):
        self.count = 0
        self.steps = 0

    # @stereotype set
    def set_count(self, count: int) -> None:
        self.count = count

    # @stereotype command
    def increment(self) -> None:
        self.count += 1
        self.steps += 1

    # @stereotype command
    def decrement(self) -> None:
        self.count -= 1
        self.steps += 1

    # @stereotype get
    def get_count(self) -> int:
        return self.count


# @stereotype boundary
class Exporter:
    def __init__(self):
        self.pages = 0

    # @stereotype get collaborator
    def get_pages(self, printer: Printer) -> int:
        return self.pages

    # @stereotype property collaborator
    def sheets(self, printer: Printer) -> int:
        return self.pages / 2

    # @stereotype set
    def set_pages(self, pages: int) -> None:
        self.pages = pages


# @stereotype factory controller
class WidgetFactory:
    def __init__(self):
        self.style = 0

    # @stereotype property factory collaborator
    def plain(self) -> Widget:
        widget = Widget(self.style)
        return widget

    # @stereotype property factory collaborator
    def bold(self) -> Widget:
        widget = Widget(self.style + 1)
        return widget

    # @stereotype property factory collaborator
    def italic(self) -> Widget:
        widget = Widget(self.style + 2)
        return widget


# @stereotype controller
class Spooler:
    def __init__(self):
        self.pages = 0

    # @stereotype controller
    def send_first(self, printer: Printer) -> None:
        printer.print_pages(self.pages)

    # @stereotype controller
    def send_second(self, printer: Printer) -> None:
        printer.print_pages(self.pages + 1)

    # @stereotype controller
    def send_third(self, printer: Printer) -> None:
        printer.print_pages(self.pages + 2)

    # @stereotype get
    def get_pages(self) -> int:
        return self.pages


# @stereotype pure-controller
class Dispatcher:
    def __init__(self):
        self.pages = 0

    # @stereotype controller
    def send_first(self, printer: Printer) -> None:
        printer.print_pages(self.pages)

    # @stereotype controller
    def send_second(self, printer: Printer) -> None:
        printer.print_pages(self.pages + 1)

    # @stereotype controller
    def send_third(self, printer: Printer) -> None:
        printer.print_pages(self.pages + 2)


# @stereotype lazy-class
class Settings:
    def __init__(self):
        self.width = 0
        self.height = 0

    # @stereotype get
    def get_width(self) -> int:
        return self.width

    # @stereotype get
    def get_height(self) -> int:
        return self.height

    # @stereotype set
    def set_width(self, width: int) -> None:
        self.width = width

    # @stereotype incidental
    def version(self) -> int:
        return 2

    # @stereotype empty
    def reload(self) -> None:
        pass


# @stereotype entity degenerate
class Plugin:
    def __init__(self):
        self.state = 0
        self.loads = 0

    # @stereotype incidental
    def version(self) -> int:
        return 1

    # @stereotype incidental
    def name(self) -> str:
        return "plugin"

    # @stereotype empty
    def unload(self) -> None:
        pass

    # @stereotype property
    def next_state(self) -> int:
        return self.state + 1

    # @stereotype command
    def reset(self) -> None:
        self.state = 0
        self.loads = 0


# @stereotype data-class
class Pair:
    def __init__(self):
        self.first = 0
        self.second = 0

    # @stereotype get
    def get_first(self) -> int:
        return self.first

    # @stereotype get
    def get_second(self) -> int:
        return self.second

    # @stereotype set
    def set_first(self, first: int) -> None:
        self.first = first


# @stereotype entity small-class
class Tank:
    def __init__(self):
        self.level = 0
        self.capacity = 0

    # @stereotype property
    def percent(self) -> int:
        return self.level * 100 / self.capacity

    # @stereotype command
    def fill(self) -> None:
        self.level = self.capacity
        self.capacity += 1


# @stereotype empty
class Tag:
    # @stereotype constructor
    def __init__(self):
        self.id = 0
