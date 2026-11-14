// Return types and return expressions (Method-Stereotypes wiki, get, predicate, and property)
import java.util.List;

class Flags {
    int count;

    // @stereotype predicate
    public Boolean isEmptyBoxed() {
        return count == 0;
    }

    // @stereotype predicate
    public boolean isEmptyPrim() {
        return count == 0;
    }
}

class Samples {
    int[] arr;
    int x;
    // @stereotype property
    int sumAt(int i) { return arr[i] + x; }
    // @stereotype predicate
    boolean isZero(int i) { return arr[i] == 0; }
    // @stereotype get
    int at(int i) { return arr[i]; }
}

// A container of Boolean values returns the container, which is no Boolean
class Switches {
    List<Boolean> states;
    int count;

    // @stereotype property
    List<Boolean> active() { return count > 0 ? states : null; }
}
