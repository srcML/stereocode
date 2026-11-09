// Locals of initializer blocks and constructors are no data members (Method-Stereotypes wiki, incidental)

import static registry.Defaults.count;

class Registry {
    private int size;
    { int count = 3; size = count; }
    static { int total = 0; }
    Registry() { int limit = 5; size = limit; }
    // count is the static import, not a data member
    // @stereotype incidental
    int count() { return count; }
}
