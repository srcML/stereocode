// A method reference of the object itself or of a type, as this::process or Demo::log, is no call, no use of the state of the object,
// and no unknown variable, while a method reference of a data member, as items::add, reads the data member

import java.util.List;
import java.util.function.Consumer;

class Demo {
    private int count;
    private int log;
    private List<Integer> items;

    // @stereotype controller stateless
    void run(List<Integer> xs) { xs.forEach(this::process); }

    // The data member log plays no part in the method reference Demo::log
    // @stereotype controller stateless
    void runStatic(List<Integer> xs) { xs.forEach(Demo::log); }

    // @stereotype controller stateless
    void runSuper(List<Integer> xs) { xs.forEach(super::equals); }

    // @stereotype collaborator incidental
    Consumer<Integer> handler() { return this::process; }

    // @stereotype controller
    void runField(List<Integer> xs) { xs.forEach(items::add); }

    // @stereotype set
    void process(int x) { count += x; }

    // @stereotype empty
    static void log(int x) { }
}
