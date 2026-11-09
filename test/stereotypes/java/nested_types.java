// Nested named types are types of their own under their qualified names, and their methods get stereotypes,
// while local and anonymous classes stay out (Class-Stereotypes wiki)

// @stereotype data-provider lazy-class small-class
class Outer {
    private int x;
    // @stereotype get
    int getX() { return x; }

    // @stereotype data-provider data-class small-class
    static class Inner { protected int v;
        // @stereotype get
        int get() { return v; } }

    // An inner class reaches the data members of the enclosing instance
    class Cursor {
        private int position;
        // @stereotype set
        void advance() { position++; }
        // @stereotype get
        int outer() { return x; }
    }

    // A record nested in a class, whose components are its data members
    record Point(int px, int py) {
        // @stereotype property
        int sum() { return px + py; }
    }

    // A nested enum, labeled with -e, and a nested interface, labeled with -i
    // @stereotype data-provider small-enum
    enum Mode { ON, OFF;
        // @stereotype predicate
        boolean isOn() { return this == ON; } }

    // @stereotype degenerate small-interface
    interface Visitor {
        // @stereotype incidental
        default int depth() { return 0; } }

    // @stereotype incidental
    void walk() { class Local { int w; int get() { return w; } } }
}

// A parent named by the qualified name of a nested type
class Derived extends Outer.Inner {
    // @stereotype get
    int peek() { return v; }
}
