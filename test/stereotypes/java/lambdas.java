// Returns inside lambdas and anonymous classes belong to them, not to the enclosing method
// (Method-Stereotypes wiki, get, property, and controller)

import java.util.function.Supplier;

class Holder {
    int value;

    // @stereotype controller
    public void register(Registry r) {
        r.add(new Supplier<Integer>() {
            public Integer get() {
                return value;
            }
        });
    }

    // @stereotype controller
    public void register2(Registry r) {
        r.add(() -> { return value; });
    }
}

// @stereotype small-class
class Clock {
    private long now;
    // @stereotype controller
    void register(Registry r) {
        r.add(new Supplier<Long>() { public Long get() { return now; } });
    }
    // @stereotype set
    void tick() { now++; }
}

class Source {
    int x;
    // @stereotype property factory collaborator
    Supplier<Integer> supplier() {
        return new Supplier<Integer>() { public Integer get() { return x; } };
    }
    // @stereotype property controller
    int lambdaCount() { Supplier<Integer> s = () -> { return x; }; return s.get() + 1; }
}
