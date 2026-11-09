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
