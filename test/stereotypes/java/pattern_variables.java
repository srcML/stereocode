// The variable of a type pattern is a local of the type that the pattern names

class Gauge {
    int level;

    // @stereotype set
    void clear() { level = 0; }
}

class Meter {
    private int level;
    private Gauge gauge;

    // @stereotype set
    void clear() { level = 0; }

    // A call on a local is a call on another object
    // @stereotype controller stateless
    void wipe(Object other) { if (other instanceof Gauge found) { found.clear(); } }

    // @stereotype controller incidental
    void zero(Object other) { if (other instanceof Gauge found) { found.level = 0; } }

    // The variable stays in scope after the statement that tests the type
    // @stereotype controller stateless
    void wipeLater(Object other) { if (!(other instanceof Gauge found)) return; found.clear(); }

    // The variable hides the data member of its name
    // @stereotype controller stateless
    void wipeOther(Object other) { if (other instanceof Gauge gauge) { gauge.clear(); } }

    // @stereotype predicate collaborator
    boolean same(Object other) { return other instanceof Meter found && found.level == level; }
}
