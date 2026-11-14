# Collaborational methods (Method-Stereotypes wiki): collaborator, controller, and wrapper

class Report:
    def __init__(self):
        self.pages = 0

    # @stereotype set collaborator
    def attach(self, printer: Printer) -> None:
        self.pages = 1

    # @stereotype controller
    def send(self, printer: Printer) -> None:
        printer.print_pages(self.pages)

    # @stereotype wrapper
    def announce(self) -> None:
        journal.write(self.pages)

    # A parameter without an annotation has an unknown type, which makes no collaborator
    # @stereotype set
    def detach(self, printer):
        self.pages = 0

    # A call on a parameter is a call on another object, whatever the type of the parameter
    # @stereotype controller
    def resend(self, printer):
        printer.print_pages(self.pages)

    # A write through a member of a parameter changes the object of the caller
    # @stereotype void-accessor controller
    def stamp(self, page):
        page.number = self.pages
