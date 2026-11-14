# Degenerate methods (Method-Stereotypes wiki): incidental, stateless, and empty

class Session:
    def __init__(self):
        self.id = 0

    # @stereotype incidental
    def version(self) -> int:
        return 3

    # @stereotype wrapper stateless
    def limit(self, value: int) -> int:
        return bounds.clamp(value)

    # @stereotype controller stateless
    def notify(self, listener) -> None:
        listener.changed()

    # A body of pass, of a docstring, or of ... holds no statement
    # @stereotype empty
    def refresh(self) -> None:
        pass

    # @stereotype empty
    def close(self):
        """Releases nothing."""

    # @stereotype empty
    def flush(self): ...

    # @stereotype empty
    def sync(self):
        """Waits for nothing."""
        # nothing to wait for
        pass
