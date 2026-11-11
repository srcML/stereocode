// A call to a method of the own type counts by the stereotypes of that method: a call to an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object and counts like a call of a free function,
// and a call to any other method may change the object

// The incidental bar is no accessor, and test, which calls bar, uses no state of the object
class Foo {
    // @stereotype incidental
    int bar() { return 4; }

    // @stereotype wrapper stateless
    boolean test() { return bar() + 5 > 0; }
}

class Cell {
    private int value;

    // @stereotype get
    int value() { return value; }

    // @stereotype empty
    void check() { }

    // @stereotype wrapper stateless
    void print() { Log.write("cell"); }

    // @stereotype property wrapper
    int checked() { check(); return value * 2; }

    // @stereotype wrapper stateless
    void refresh() { this.check(); }

    // @stereotype property wrapper
    int printed() { print(); return value() + 1; }
}
