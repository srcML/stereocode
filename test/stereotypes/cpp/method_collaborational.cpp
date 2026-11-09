// Collaborational methods (Method-Stereotypes wiki): collaborator, controller, and wrapper

void log(int pages);

class Printer {
public:
    void print(int pages);
};

class Report {
    int m_pages;
public:
    // @stereotype set collaborator
    void attach(Printer* printer) { m_pages = 1; }

    // @stereotype controller
    void send(Printer& printer) { printer.print(m_pages); }

    // @stereotype wrapper
    void announce() const { log(m_pages); }
};
