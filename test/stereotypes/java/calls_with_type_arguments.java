// srcML marks the object before the explicit type arguments of a call apart from the call, which still names its object
// (Method-Stereotypes wiki, Calls)
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

class Shelf extends ArrayList<String> {
    private List<String> labels;

    // A call through the name of a type is a call of a free function, also in a class with a parent outside the archive
    // @stereotype wrapper stateless
    Object none() { return Collections.<String>emptyList(); }

    // @stereotype command
    void label() { labels.<String>add("new"); }

    // @stereotype property
    int labelCount() { return this.labels.<String>size(); }

    // @stereotype command
    void fill() { this.<String>add("first"); }

    // @stereotype controller stateless
    Object firstOf(Shelf other) { return other.copy().<String>get(0); }
}

class Rack {
    private List<String> labels;

    // @stereotype command
    void label() { labels.<String>add("new"); }

    // @stereotype property
    int labelCount() { return this.labels.<String>size(); }
}
