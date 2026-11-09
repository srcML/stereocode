// Large and lazy structures (run with -i for interfaces and -e for enums): the class rules name the structure,
// as in large-interface, lazy-enum, and large-record (Class-Stereotypes wiki, large-class and lazy-class)
// The class Widget and the records Track and Trail have 0, 4, and 2 methods, which give the threshold 2.00 + 1.63 = 3.63

class Widget {
}

// An interface has no data members, so its default methods return the object itself or call its abstract methods
// @stereotype large-interface
interface Shape {
    void clear();

    // @stereotype get
    default Shape self() { return this; }

    // The abstract clear has no stereotypes, so its call may change the object
    // @stereotype command
    default void reset() { clear(); }

    // The call of the accessor self reads the object
    // @stereotype controller
    default void paint(Widget widget) { widget.poke(self()); }

    // @stereotype factory collaborator stateless
    default Widget make() { return new Widget(); }
}

// @stereotype data-provider lazy-interface
interface Lazy {
    // @stereotype get
    default Lazy self() { return this; }

    // @stereotype get
    default Lazy current() { return this; }

    // @stereotype empty
    default void reset() { }

    // @stereotype empty
    default void clear() { }
}

// Four methods, more than the threshold, with a get, a set, a controller, and a factory in balance
// @stereotype large-enum
enum Mode {
    FAST, SLOW;

    private int level;

    // @stereotype get
    int getLevel() { return level; }

    // @stereotype set
    void setLevel(int value) { level = value; }

    // @stereotype controller
    void paint(Widget widget) { widget.poke(level); }

    // @stereotype factory collaborator stateless
    Widget make() { return new Widget(); }
}

// Two of the four methods are degenerate, more than one third, and the others are a get and a set
// @stereotype lazy-enum
enum Phase {
    ON, OFF;

    private int count;

    // @stereotype get
    int getCount() { return count; }

    // @stereotype set
    void setCount(int value) { count = value; }

    // @stereotype empty
    void reset() { }

    // @stereotype empty
    void clear() { }
}

// The fields of a record are final, so a call on the list items is the mutator
// @stereotype large-record
record Track(int width, java.util.List<String> items) {
    // @stereotype get
    int getWidth() { return width; }

    // @stereotype command
    void add(String item) { items.add(item); }

    // @stereotype controller
    void paint(Widget widget) { widget.poke(width); }

    // @stereotype factory collaborator stateless
    Widget make() { return new Widget(); }
}

// @stereotype data-provider lazy-record small-record
record Trail(int length) {
    // @stereotype get
    int getLength() { return length; }

    // @stereotype empty
    void clear() { }
}
