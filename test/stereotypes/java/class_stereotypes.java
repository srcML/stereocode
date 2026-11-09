// Class stereotypes (Class-Stereotypes wiki, rules for class stereotype identification)
// The method labels are listed to show the class signature; constructors do not count

// @stereotype entity boundary
class Measure {
    private int value;
    private int changes;

    // @stereotype property collaborator
    int scaled(Scale scale) { return value * 2; }

    // @stereotype command collaborator
    void adjust(Scale scale) { value *= 2; changes++; }

    // @stereotype get
    int value() { return value; }
}

// @stereotype minimal-entity boundary
class Gauge {
    private int level;
    private int limit;

    // @stereotype get collaborator
    int level(Scale scale) { return level; }

    // @stereotype set collaborator
    void setLevel(int amount, Scale scale) { level = amount; }

    // @stereotype command
    void fill() { level = limit; limit++; }
}

// @stereotype data-provider
class Circle {
    private double radius;

    // @stereotype get
    double radius() { return radius; }

    // @stereotype predicate
    boolean isUnit() { return radius == 1.0; }

    // @stereotype property
    double area() { return 3.14159 * radius * radius; }

    // @stereotype set
    void setRadius(double r) { radius = r; }
}

// @stereotype minimal-entity commander
class Counter {
    private int count;
    private int steps;

    // @stereotype set
    void setCount(int c) { count = c; }

    // @stereotype command
    void increment() { count++; steps++; }

    // @stereotype command
    void decrement() { count--; steps++; }

    // @stereotype get
    int count() { return count; }
}

// @stereotype boundary
class Exporter {
    private int pages;

    // @stereotype get collaborator
    int pages(Printer printer) { return pages; }

    // @stereotype property collaborator
    int sheets(Printer printer) { return pages / 2; }

    // @stereotype set
    void setPages(int p) { pages = p; }
}

// @stereotype factory controller
class WidgetFactory {
    private int style;

    // @stereotype property factory collaborator
    Widget plain() { Widget widget = new Widget(style); return widget; }

    // @stereotype property factory collaborator
    Widget bold() { Widget widget = new Widget(style + 1); return widget; }

    // @stereotype property factory collaborator
    Widget italic() { Widget widget = new Widget(style + 2); return widget; }
}

// @stereotype controller
class Spooler {
    private int pages;

    // @stereotype controller
    void sendFirst(Printer printer) { printer.print(pages); }

    // @stereotype controller
    void sendSecond(Printer printer) { printer.print(pages + 1); }

    // @stereotype controller
    void sendThird(Printer printer) { printer.print(pages + 2); }

    // @stereotype get
    int pages() { return pages; }
}

// @stereotype pure-controller
class Dispatcher {
    private int pages;

    // @stereotype controller
    void sendFirst(Printer printer) { printer.print(pages); }

    // @stereotype controller
    void sendSecond(Printer printer) { printer.print(pages + 1); }

    // @stereotype controller
    void sendThird(Printer printer) { printer.print(pages + 2); }
}

// @stereotype lazy-class
class Settings {
    private int width;
    private int height;

    // @stereotype get
    int width() { return width; }

    // @stereotype get
    int height() { return height; }

    // @stereotype set
    void setWidth(int w) { width = w; }

    // @stereotype incidental
    int version() { return 2; }

    // @stereotype empty
    void reload() { }
}

// @stereotype entity degenerate
class Plugin {
    private int state;
    private int loads;

    // @stereotype incidental
    int version() { return 1; }

    // @stereotype incidental
    String name() { return "plugin"; }

    // @stereotype empty
    void unload() { }

    // @stereotype property
    int state() { return state + 1; }

    // @stereotype command
    void reset() { state = 0; loads = 0; }
}

// @stereotype data-class
class Pair {
    private int first;
    private int second;

    // @stereotype get
    int first() { return first; }

    // @stereotype get
    int second() { return second; }

    // @stereotype set
    void setFirst(int f) { first = f; }
}

// @stereotype entity small-class
class Tank {
    private int level;
    private int capacity;

    // @stereotype property
    int percent() { return level * 100 / capacity; }

    // @stereotype command
    void fill() { level = capacity; capacity++; }
}

// @stereotype empty
class Tag {
    private int id;

    // @stereotype constructor
    Tag() { id = 0; }
}
