// Degenerate methods (Method-Stereotypes wiki): incidental, stateless, and empty

class Session {
    constructor() {
        this.id = 0;
    }

    // @stereotype incidental
    version() {
        return 3;
    }

    // @stereotype wrapper stateless
    limit(value) {
        return bounds.clamp(value);
    }

    // @stereotype controller stateless
    notify(listener) {
        listener.changed();
    }

    // @stereotype empty
    refresh() {}

    // A body of comments holds no statement
    // @stereotype empty
    close() {
        // nothing to release
    }
}
