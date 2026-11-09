// Class stereotypes (Class-Stereotypes wiki, rules for class stereotype identification)
// The method labels are listed to show the class signature; constructors do not count

class Scale;
class Printer {
public:
    void print(int pages);
};
class Widget {
public:
    explicit Widget(int style);
};

// @stereotype entity boundary
class Measure {
    int m_value;
    int m_changes;
public:
    // @stereotype property collaborator
    int scaled(const Scale& scale) const { return m_value * 2; }

    // @stereotype command collaborator
    void adjust(const Scale& scale) { m_value *= 2; m_changes++; }

    // @stereotype get
    int value() const { return m_value; }
};

// @stereotype minimal-entity boundary
class Gauge {
    int m_level;
    int m_limit;
public:
    // @stereotype get collaborator
    int level(const Scale& scale) const { return m_level; }

    // @stereotype set collaborator
    void setLevel(int level, const Scale& scale) { m_level = level; }

    // @stereotype command
    void fill() { m_level = m_limit; m_limit++; }
};

// @stereotype data-provider
class Circle {
    double m_radius;
public:
    // @stereotype get
    double radius() const { return m_radius; }

    // @stereotype predicate
    bool isUnit() const { return m_radius == 1.0; }

    // @stereotype property
    double area() const { return 3.14159 * m_radius * m_radius; }

    // @stereotype set
    void setRadius(double radius) { m_radius = radius; }
};

// @stereotype minimal-entity commander
class Counter {
    int m_count;
    int m_steps;
public:
    // @stereotype set
    void setCount(int count) { m_count = count; }

    // @stereotype command
    void increment() { m_count++; m_steps++; }

    // @stereotype command
    void decrement() { m_count--; m_steps++; }

    // @stereotype get
    int count() const { return m_count; }
};

// @stereotype boundary
class Exporter {
    int m_pages;
public:
    // @stereotype get collaborator
    int pages(const Printer& printer) const { return m_pages; }

    // @stereotype property collaborator
    int sheets(const Printer& printer) const { return m_pages / 2; }

    // @stereotype set
    void setPages(int pages) { m_pages = pages; }
};

// @stereotype factory controller
class WidgetFactory {
    int m_style;
public:
    // @stereotype property factory collaborator
    Widget* plain() const { Widget* widget = new Widget(m_style); return widget; }

    // @stereotype property factory collaborator
    Widget* bold() const { Widget* widget = new Widget(m_style + 1); return widget; }

    // @stereotype property factory collaborator
    Widget* italic() const { Widget* widget = new Widget(m_style + 2); return widget; }
};

// @stereotype controller
class Spooler {
    int m_pages;
public:
    // @stereotype controller
    void sendFirst(Printer& printer) { printer.print(m_pages); }

    // @stereotype controller
    void sendSecond(Printer& printer) { printer.print(m_pages + 1); }

    // @stereotype controller
    void sendThird(Printer& printer) { printer.print(m_pages + 2); }

    // @stereotype get
    int pages() const { return m_pages; }
};

// @stereotype pure-controller
class Dispatcher {
    int m_pages;
public:
    // @stereotype controller
    void sendFirst(Printer& printer) { printer.print(m_pages); }

    // @stereotype controller
    void sendSecond(Printer& printer) { printer.print(m_pages + 1); }

    // @stereotype controller
    void sendThird(Printer& printer) { printer.print(m_pages + 2); }
};

// @stereotype lazy-class
class Settings {
    int m_width;
    int m_height;
public:
    // @stereotype get
    int width() const { return m_width; }

    // @stereotype get
    int height() const { return m_height; }

    // @stereotype set
    void setWidth(int width) { m_width = width; }

    // @stereotype incidental
    int version() const { return 2; }

    // @stereotype empty
    void reload() { }
};

// @stereotype entity degenerate
class Plugin {
    int m_state;
    int m_loads;
public:
    // @stereotype incidental
    int version() const { return 1; }

    // @stereotype incidental
    const char* name() const { return "plugin"; }

    // @stereotype empty
    void unload() { }

    // @stereotype property
    int state() const { return m_state + 1; }

    // @stereotype command
    void reset() { m_state = 0; m_loads = 0; }
};

// @stereotype data-class
class Pair {
    int m_first;
    int m_second;
public:
    // @stereotype get
    int first() const { return m_first; }

    // @stereotype get
    int second() const { return m_second; }

    // @stereotype set
    void setFirst(int first) { m_first = first; }
};

// @stereotype entity small-class
class Tank {
    int m_level;
    int m_capacity;
public:
    // @stereotype property
    int percent() const { return m_level * 100 / m_capacity; }

    // @stereotype command
    void fill() { m_level = m_capacity; m_capacity++; }
};

// @stereotype empty
class Tag {
    int m_id;
public:
    // @stereotype constructor
    Tag() : m_id(0) {}
};
