// A call on a data member whose type is a type of the archive counts by the stereotypes of the called method of that type:
// a call to an accessor reads the data member, a call to a degenerate method counts like a call of a free function,
// and a call to any other method may change the data member

import java.util.List;

class Engine {
    private int rpm;

    // @stereotype get
    int rpm() { return rpm; }

    // @stereotype set
    void start() { rpm = 800; }

    // @stereotype wrapper stateless
    void log() { Log.write("engine"); }
}

interface Gauge {
    int read();
}

class Car {
    private Engine engine;
    private Gauge gauge;
    private List<Integer> trips;

    // @stereotype property collaborator
    int speed() { return engine.rpm() / 100; }

    // @stereotype property collaborator
    int ownSpeed() { return this.engine.rpm() / 100; }

    // @stereotype command collaborator
    void go() { engine.start(); }

    // @stereotype wrapper
    void report() { engine.log(); }

    // A method that the type of the data member declares without a body has no stereotypes and may change the data member
    // @stereotype property non-void-command collaborator
    int level() { return gauge.read(); }

    // A call on a data member whose type is no type of the archive may change the data member
    // @stereotype command
    void trip(int length) { trips.add(length); }
}
