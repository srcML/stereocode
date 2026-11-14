// The body of an enum constant defines methods of its own, and each of them is a method of the enum

class Reader {
    private int position;

    // @stereotype set
    void advance() { position++; }
}

// Each of the three constants defines the method read, and three methods are no small enum
// @stereotype degenerate pure-controller
enum State {
    Start {
        // @stereotype controller stateless
        void read(Reader reader) { reader.advance(); }
    },
    Middle {
        // @stereotype controller stateless
        void read(Reader reader) { reader.advance(); }
    },
    End {
        // @stereotype controller stateless
        void read(Reader reader) { reader.advance(); }
    };

    abstract void read(Reader reader);
}

// A call names the method that the enum declares or defines, never a method of that name in the body of a constant, which overrides it
enum Operation {
    PLUS {
        // @stereotype incidental
        int apply(int first, int second) { return first + second; }
    },
    MINUS {
        // @stereotype incidental
        int apply(int first, int second) { return first - second; }
    };

    // The abstract apply has no stereotypes, so its call may change the object
    abstract int apply(int first, int second);

    // @stereotype non-void-command
    int twice(int value) { return apply(value, value); }
}

enum Level {
    LOW,
    HIGH {
        // @stereotype incidental
        int weight() { return 2; }
    };

    int base;

    // @stereotype get
    int weight() { return base; }

    // The call reaches the get of the enum alone
    // @stereotype property
    int doubled() { return weight() * 2; }
}

// A method that only the body of a constant defines is reached by the calls of that body
enum Shape {
    SQUARE {
        // @stereotype property
        int area() { return side() * side(); }

        // @stereotype get
        int side() { return size; }
    };

    int size;

    abstract int area();
}
