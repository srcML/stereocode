// Calls on call results and casts are calls on other objects (Method-Stereotypes wiki, controller)

class Pipe {
    // @stereotype controller stateless
    void close(Object o) { ((java.io.Closeable) o).close(); }
    // @stereotype controller stateless
    void run() { Flowable.just(1).subscribe(); }
    // A message to the error stream is an ignored call
    // @stereotype incidental
    void warn(int v) { System.err.println(v); }
}

// A call of a query method on a data member reads the data member
class Inventory {
    private List<Integer> items;
    // @stereotype property
    int total() { return items.size(); }
    // @stereotype predicate
    boolean has(Integer x) { return this.items.contains(x); }
    // @stereotype command
    void add(Integer x) { items.add(x); }
}
