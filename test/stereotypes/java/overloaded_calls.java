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

// A call whose number of arguments only a static method of its name takes is a call of that static method, which is a free function
class Label {
    private String key;

    static boolean isData(String key) { return key != null; }

    // @stereotype predicate wrapper
    boolean isData() { return isData(key); }

    static int width(int margin, int border) { return margin + border; }

    // @stereotype set
    void width(int value) { key = "w" + value; }

    // The call with one argument reaches the method
    // @stereotype command
    void widen() { width(4); }

    // @stereotype wrapper stateless
    int padded() { return width(1, 2); }
}

class ShortLabel extends Label {
    // A static method of a parent is reached as well
    // @stereotype wrapper stateless
    int padded(int margin) { return width(margin, 1) + 1; }
}

// A call whose number of arguments only a method without a definition takes reaches no method with stereotypes and may thus change the object
abstract class Timer {
    abstract int schedule(int task, long delay, int unit);

    // The call names the abstract method, whose definitions lie in other classes, never the calling method itself
    // @stereotype non-void-command
    int schedule(int task) { return schedule(task, 0L, 1); }

    static int width(int margin, int border) { return margin + border; }

    abstract int width(String text, String font);

    // The abstract method takes two arguments as the static method does
    // @stereotype non-void-command
    int padded() { return width("a", "b"); }

    // The call with one argument names the method equals of the class Object, which reads the object
    // @stereotype predicate
    boolean equals(int first, int second) { return first == second && equals(null); }

    abstract int delay(int task);

    // The call names the abstract method, which takes another type than the calling method
    // @stereotype non-void-command
    int delay(long task) { return delay((int) task); }
}

abstract class Alarm extends Timer {
    private int limit;

    // The abstract method of the parent takes other types than the calling method
    // @stereotype non-void-command
    int schedule(String task, long delay, int unit) { return schedule(0, delay, unit); }

    // The method defines the abstract method of the parent, and its call is a call to itself
    // @stereotype property
    int width(String text, String font) { return text == font ? limit : width(font, font); }
}

// Where no method of the name takes the arguments of a call, a parent outside the archive may define the called method
class Names extends Roster {
    // @stereotype non-void-command
    boolean add(String name) { add(0, name); return true; }

    // The call without arguments names the query method size of the parent, which reads the object
    // @stereotype property
    int size(int scale) { return size() * scale; }

    // The parent may define the called method as well, which takes one argument as the calling method does
    // @stereotype command
    void append(long count) { append((int) count); }
}
