// Collaborational methods (Method-Stereotypes wiki): collaborator, controller, and wrapper

class Report {
    private int pages;

    // @stereotype set collaborator
    public void attach(Printer printer) { pages = 1; }

    // @stereotype controller
    public void send(Printer printer) { printer.print(pages); }

    // @stereotype wrapper
    public void announce() { Journal.write(pages); }
}
