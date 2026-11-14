// Static data members are globals, so a method that reads or writes one uses no state of the object
// (Method-Stereotypes wiki, requestFusion, whose ASYNC is an interface constant in RxJava)

abstract class BasicQueueSubscription<T> {
    static final int ASYNC = 2;

    // @stereotype incidental
    public final int requestFusion(int mode) {
        return mode & ASYNC;
    }
}

class Sequence {
    static int n;
    // @stereotype incidental
    void inc() { n++; }
    // @stereotype incidental
    void reset() { Sequence.n = 0; }
}

// A parent outside the archive may declare a written name, but not a static data member of the type
class Recorder extends Journal {
    static int hits;
    // @stereotype incidental
    void hit() { hits++; }
    // @stereotype set
    void mark() { marked = true; }
}

// A static data member of a parent is a global as well
class Sequel extends Sequence {
    // @stereotype incidental
    void next() { n++; }
}

// A static data member that its type qualifies is no parameter of the same name
class Depot {
    static Crate shared;

    // @stereotype collaborator incidental
    void keep(Crate shared) { Depot.shared = shared; }

    // @stereotype collaborator incidental
    void hold(Crate other) { Depot.shared = other; }
}
