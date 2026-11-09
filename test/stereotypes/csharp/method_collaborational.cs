// Collaborational methods (Method-Stereotypes wiki): collaborator, controller, and wrapper

public class Report
{
    private int pages;

    // @stereotype set collaborator
    public void Attach(Printer printer) { pages = 1; }

    // @stereotype controller
    public void Send(Printer printer) { printer.Print(pages); }

    // @stereotype wrapper
    public void Announce() { Journal.Write(pages); }
}
