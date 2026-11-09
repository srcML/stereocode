// Calls to overloaded methods of the own type: a call reaches the overloads of its name that take as many arguments as the call passes

class Gauge {
    private double level;
    private int offset;

    // @stereotype get
    double level() { return level; }

    // @stereotype set
    void level(double value) { level = value; }

    // The call without arguments reaches the get alone, while the set takes one argument
    // @stereotype property
    double doubled() { return level() * 2; }

    // @stereotype property
    int scale(int unit) { return offset * unit; }

    // @stereotype set
    void scale(double factor) { level = level * factor; }

    // Both overloads take one argument, and the set may change the object
    // @stereotype property non-void-command
    int scaled() { return scale(2) + 1; }

    // @stereotype get
    int total(int... values) { return offset; }

    // @stereotype set
    void total(int first, int second) { offset = first + second; }

    // A call with three arguments reaches the variadic overload alone
    // @stereotype property
    int summed() { return total(1, 2, 3) + 1; }
}
