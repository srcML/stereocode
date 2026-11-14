# Strings: srcML marks an f-string as one literal, so the names in its braces are read from its text

class Label:
    def __init__(self):
        self.text = ""
        self.width = 0

    # @stereotype property
    def render(self) -> str:
        return f"[{self.text}]"

    # A string without the prefix f holds no names
    # @stereotype incidental
    def template(self) -> str:
        return "[{self.text}]"

    # Doubled braces hold no name
    # @stereotype incidental
    def braces(self) -> str:
        return f"{{self.text}}"

    # A format after the colon may hold a name of its own
    # @stereotype property
    def padded(self) -> str:
        return F'{self.text:>{self.width}}'

    # A name of an f-string that is no data member uses no state
    # @stereotype incidental
    def greeting(self, name: str) -> str:
        return f"Hello, {name}!"


# @stereotype property
def describe(count: int) -> str:
    return f"{count} items"


# @stereotype literal
def banner() -> str:
    return f"{'=' * 20}"
