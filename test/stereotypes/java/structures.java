// Structures other than classes (run with -i for interfaces and -e for enums): the class rules name
// the structure, as in small-interface, small-enum, and small-record

// @stereotype commander small-interface
interface Shape {
    int area();

    // The abstract area has no stereotypes, so its call may change the object and reads no state
    // @stereotype non-void-command
    default int twice() { return area() * 2; }
}

// @stereotype degenerate small-enum
enum Color {
    RED, GREEN;

    // @stereotype incidental
    int code() { return 1; }
}

// @stereotype degenerate small-record
record Pair(int left, int right) {
    // @stereotype incidental
    int zero() { return 0; }
}

// An interface has no data members, so a default method that returns the object itself is its get
// @stereotype data-provider data-interface small-interface
interface Chain {
    // @stereotype get
    default Chain self() { return this; }
}

// The constructor does not count, so the get is the only method
// @stereotype data-provider data-enum small-enum
enum Level {
    LOW(1), HIGH(2);

    private final int code;

    Level(int code) { this.code = code; }

    // @stereotype get
    int code() { return code; }
}

// @stereotype data-provider data-record small-record
record Size(int width) {
    // @stereotype get
    public int width() { return width; }
}
