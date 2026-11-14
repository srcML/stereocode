// Record components are private final fields (Java Language Specification 8.10.3)
// (Method-Stereotypes wiki, get and property)

interface Shape {
    double area();
    default double twice() { return 2 * area(); }
    static Shape unit() { return null; }
}
// @stereotype data-provider small-record
record Pair(int left, int right) {
    // @stereotype property
    int sum() { return left + right; }
}

record Point(int x, int y) {
    // @stereotype property
    public int sum() { return x + y; }
    // @stereotype get
    public int getX() { return x; }
}

// Each component of a record has an accessor of its name, which the record may define, and a call of the accessor reads the object
record Span(int low, int high) {
    // @stereotype get
    public int low() { return low; }

    // @stereotype predicate
    boolean wide() { return high() > 10; }

    // @stereotype predicate
    boolean wideThroughObject() { return this.high() > 10; }

    // @stereotype predicate
    boolean wideField() { return high > 10; }

    // @stereotype property
    int length() { return high() - low(); }

    // No component has the name of the called method
    // @stereotype wrapper stateless
    int middle() { return center(); }
}

class Badge {
    private Span span;

    // @stereotype property collaborator
    int low() { return span.low() + 1; }

    // @stereotype property collaborator
    int high() { return span.high() + 1; }

    // The accessor of a component takes no argument
    // @stereotype property non-void-command collaborator
    int shifted() { return span.high(2) + 1; }
}
