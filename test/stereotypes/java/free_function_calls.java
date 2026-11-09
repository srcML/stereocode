// A call on a parameter of a type that is not primitive changes the object of the caller, unless it calls a query method,
// and a call on a static field changes a global (Free-Function-Stereotypes wiki, command, global-command, and wrapper)
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

final class Lists {
    static final AtomicInteger COUNTER = new AtomicInteger();
    private static final String PREFIX = "p";

    // @stereotype command
    static void fill(List<Integer> items, int value) { items.add(value); }

    // A query method reads the parameter
    // @stereotype property wrapper
    static int first(List<Integer> items) { return items.get(0); }

    // A String is primitive, so the parameter holds no object of the caller
    // @stereotype property wrapper
    static String trimmed(String text) { return text.trim(); }

    // @stereotype global-command wrapper
    static void tick(int times) { COUNTER.addAndGet(times); }

    // A call on a final static field of a primitive type or String changes no global
    // @stereotype wrapper
    static String upper() { return PREFIX.toUpperCase(); }
}
