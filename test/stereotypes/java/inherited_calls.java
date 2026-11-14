// A call to an inherited method counts by the stereotypes of that method, also through super

class Sensor {
    protected double reference;

    // @stereotype get
    double reference() { return reference; }

    // @stereotype incidental
    int version() { return 2; }

    // @stereotype set
    void calibrate(double value) { reference = value; }
}

class Probe extends Sensor {
    private double reading;

    // @stereotype property
    double offset() { return reading - reference(); }

    // @stereotype property
    double scaled() { return super.reference() * 2; }

    // @stereotype wrapper stateless
    int doubledVersion() { return version() * 2; }

    // A call to an inherited set may change the object
    // @stereotype command
    void reset() { calibrate(0); }
}

class Counter {
    protected int count;
    protected int reads;

    // @stereotype get set
    int count() { reads++; return count; }
}

class QuietCounter extends Counter {
    // A method with the signature of a method of the parent replaces that method in the calls of the child
    // @stereotype get
    int count() { return count; }

    // @stereotype property
    int doubled() { return count() * 2; }

    // A call through super reaches the method of the parent
    // @stereotype property non-void-command
    int counted() { return super.count() * 2; }
}

// A call through super after the name of an interface names a default method of that interface
interface Greeter {
    String name();

    // The abstract name has no stereotypes, so its call may change the object
    // @stereotype non-void-command
    default String greet() { return "Hello, " + name(); }

    // @stereotype incidental
    default int limit() { return 3; }
}

class Robot implements Greeter {
    private String model;

    // @stereotype get
    public String name() { return model; }

    // @stereotype non-void-command
    public String greet() { return Greeter.super.greet() + "!"; }

    // A call of a method without a use of the object counts as a call of a free function
    // @stereotype wrapper stateless
    public int limit() { return Greeter.super.limit() + 1; }
}
