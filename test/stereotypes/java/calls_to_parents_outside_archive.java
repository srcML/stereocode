// A call to the object itself that names no method of the archive names a method of a parent outside the archive, since Java has no
// free functions: a query method reads the object, and any other method may change it (Method-Stereotypes wiki, Calls)
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

class Names extends ArrayList<String> {
    // @stereotype command
    void reset() { clear(); }

    // @stereotype command
    void drop() { super.clear(); }

    // @stereotype property
    String first() { return size() > 0 ? get(0) : null; }

    // @stereotype predicate
    boolean none() { return isEmpty(); }

    // @stereotype property
    int total() { return this.size(); }

    // A write through the result of a query method changes the object
    // @stereotype command
    void trim() { get(0).value = 0; }

    static int twice(int value) { return 2 * value; }

    // A static method of the type is a free function
    // @stereotype wrapper stateless
    int four() { return twice(2); }

    class Cursor {
        // A static method of a type around the type is a free function as well
        // @stereotype wrapper stateless
        int eight() { return twice(4); }
    }
}

class Ticker extends AtomicInteger {
    private int resets;

    // @stereotype command
    void tick() { incrementAndGet(); }

    // @stereotype command
    void reset() { resets++; set(0); }

    // @stereotype property
    int current() { return get(); }
}

// Every class inherits the accessors of Object, which read the object
class Point {
    private int x;
    private String name;

    // @stereotype property
    @Override public String toString() { return super.toString() + "[" + name + "]"; }

    // @stereotype property
    @Override public int hashCode() { return 31 * super.hashCode() + x; }

    // @stereotype predicate collaborator
    @Override public boolean equals(Object other) { return super.equals(other) && x == ((Point) other).x; }

    // @stereotype property
    public Point copy() throws CloneNotSupportedException { return (Point) super.clone(); }

    // @stereotype property
    String describe() { return this.toString(); }

    // @stereotype predicate controller
    boolean sameClass(Object other) { return other != null && getClass() == other.getClass(); }

    // Any other name stays a call of a free function in a class whose parents the archive holds
    // @stereotype wrapper stateless
    void pause() throws InterruptedException { wait(); }
}

class Labeled extends Point {
    private String label;

    // @stereotype property
    @Override public String toString() { return super.toString() + label; }

    // @stereotype property
    int code() { return super.hashCode(); }
}
