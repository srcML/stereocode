// The creation of an object of an anonymous class is a constructor call, as the creation of an object of a named class is

class Maker {
    // @stereotype factory collaborator stateless
    Runnable named() { return new Thread(); }

    // @stereotype factory collaborator stateless
    Runnable anonymous() { return new Runnable() { public void run() { } }; }

    // @stereotype collaborator stateless
    void local() { Object watcher = new Object() { }; }

    // @stereotype collaborator stateless
    void localNamed() { Object watcher = new Object(); }
}

// Inside an anonymous or a local class, this and super name the object of that class, never the object of the method around the class
class Hub {
    private int count;

    // The data member count of the anonymous class hides the data member of Hub
    // @stereotype factory collaborator stateless
    Runnable counter() { return new Runnable() { private int count; public void run() { this.count++; } }; }

    // @stereotype controller stateless
    void watch(Registry registry) { registry.add(new Thread() { @Override public void run() { super.run(); } }); }

    // The call names the method reset of the anonymous class
    // @stereotype controller stateless
    void arm(Registry registry) { registry.add(new Timer() { void fire() { reset(); } void reset() { } }); }

    // @stereotype controller stateless
    void listen(Registry registry) { registry.add(new Object() { void done() { registry.remove(this); } }); }

    // The class Hub qualifies the object of the method
    // @stereotype controller
    void enlist(Registry registry) { registry.add(new Object() { void done() { registry.remove(Hub.this); } }); }

    // The call reaches the method reset of Hub, which the anonymous class does not define
    // @stereotype command collaborator
    void clear(Registry registry) { registry.add(new Object() { void done() { reset(); } }); }

    // The local class defines a data member and a method under the names of those of Hub
    // @stereotype controller stateless
    void drain(Registry registry) {
        class Drain {
            private int count;
            void run() { this.count--; reset(); }
            void reset() { }
        }
        registry.add(new Drain());
    }

    // @stereotype set
    void reset() { count = 0; }
}
