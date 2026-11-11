// Degenerate methods (Method-Stereotypes wiki): incidental, stateless, and empty

int clamp(int value);

class Listener {
public:
    void changed();
};

class Session {
    int m_id;
public:
    // @stereotype incidental
    int version() const { return 3; }

    // @stereotype wrapper stateless
    int limit(int value) const { return clamp(value); }

    // @stereotype controller stateless
    void notify(Listener& listener) const { listener.changed(); }

    // @stereotype empty
    void refresh() { }

    // @stereotype empty
    void close() { /* nothing to release */ }
};
