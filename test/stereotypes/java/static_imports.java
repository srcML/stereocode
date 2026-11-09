// A static field that a static import names is a global, which no parent outside the archive declares as a data member
// (Method-Stereotypes wiki, set and incidental)
import static shop.Settings.level;

class Settings {
    static int level;
}

class Basket extends Collection {
    int count;
    // @stereotype incidental
    void raise() { level = 2; }
    // @stereotype set
    void add(int n) { count = n; }
    // @stereotype set
    void mark() { marked = true; }
}

class Store {
    // @stereotype global-command
    static void configure(int value) { Settings.level = value; }
}

// A name without a qualifier reaches the static fields of the types around it, of their ancestors, and of static imports
class Verbs {
    static final String SKIP = "skip";
}

class Checks {
    // SKIP of Verbs lies out of reach
    // @stereotype literal wrapper
    static void check() { Journal.write(SKIP.trim()); }
}
