// Collaborational methods (Method-Stereotypes wiki): collaborator, controller, and wrapper

class Report {
    private pages = 0;

    // @stereotype set collaborator
    attach(printer: Printer): void {
        this.pages = 1;
    }

    // @stereotype controller
    send(printer: Printer): void {
        printer.printPages(this.pages);
    }

    // A call on a name that the file does not declare calls a function of a module or of the environment
    // @stereotype wrapper
    announce(): void {
        journal.write(this.pages);
    }

    // A parameter without an annotation has an unknown type, which makes no collaborator
    // @stereotype set
    detach(printer) {
        this.pages = 0;
    }

    // A call on a parameter is a call on another object, whatever the type of the parameter
    // @stereotype controller
    resend(printer) {
        printer.printPages(this.pages);
    }

    // A write through a member of a parameter changes the object of the caller
    // @stereotype void-accessor controller
    stamp(page) {
        page.number = this.pages;
    }
}

class Spool {
    private printer: Printer;
    private pages: number;

    // @stereotype constructor
    constructor(printer: Printer, pages: number) {
        this.printer = printer;
        this.pages = pages;
    }

    // The class of a data member makes a collaborator, and a call on the data member may change the data member
    // @stereotype command collaborator
    flush(): void {
        this.printer.printPages(this.pages);
    }

    // @stereotype get collaborator
    current(): Printer {
        return this.printer;
    }
}
