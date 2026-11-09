// Degenerate methods (Method-Stereotypes wiki): incidental, stateless, and empty

class Session {
    private int id;

    // @stereotype incidental
    public int version() { return 3; }

    // @stereotype wrapper stateless
    public int limit(int value) { return Bounds.clamp(value); }

    // @stereotype controller stateless
    public void notify(Listener listener) { listener.changed(); }

    // @stereotype empty
    public void refresh() { }

    // @stereotype empty
    public void close() { /* nothing to release */ }
}
