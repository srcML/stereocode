// A static import of a type outside the archive may name any function, so a call without an object stays a call of a free function,
// also in a class with a parent outside the archive (Method-Stereotypes wiki, Calls)
import static java.util.Objects.requireNonNull;
import java.util.ArrayList;

class Entries extends ArrayList<String> {
    // @stereotype wrapper stateless
    void check(String name) { requireNonNull(name); }

    // A call through the object names a method of the object
    // @stereotype command
    void reset() { this.clear(); }

    // @stereotype property
    int total() { return this.size(); }
}
