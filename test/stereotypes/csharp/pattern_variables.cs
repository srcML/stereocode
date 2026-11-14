// The variable of a type pattern is a local of the type that the pattern names

class Gauge {
    public int level;

    // @stereotype set
    public void Clear() { level = 0; }
}

class Meter {
    private int level;
    private Gauge gauge;

    // @stereotype set
    void Clear() { level = 0; }

    // A call on a local is a call on another object
    // @stereotype controller stateless
    void Wipe(object other) { if (other is Gauge found) { found.Clear(); } }

    // @stereotype controller incidental
    void Zero(object other) { if (other is Gauge found) { found.level = 0; } }

    // The variable stays in scope after the statement that tests the type
    // @stereotype controller stateless
    void WipeLater(object other) { if (!(other is Gauge found)) return; found.Clear(); }

    // A negated pattern declares its variable as well
    // @stereotype controller stateless
    void WipeUnless(object other) { if (other is not Gauge found) return; found.Clear(); }

    // The variable hides the data member of its name
    // @stereotype controller stateless
    void WipeOther(object other) { if (other is Gauge gauge) { gauge.Clear(); } }

    // @stereotype controller stateless
    void WipeCase(object other) { switch (other) { case Gauge found: found.Clear(); break; } }

    // The words not and or of a pattern name no type and no variable
    // @stereotype command collaborator
    void WipeOwn(object other) { if (other is not Meter) { gauge.Clear(); } }

    // @stereotype predicate collaborator
    bool Same(object other) { return other is Meter found && found.level == level; }
}
