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
