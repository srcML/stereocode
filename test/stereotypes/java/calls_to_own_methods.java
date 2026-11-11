// Calls to the methods of the own type: a call to a method labeled as an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object, and a call to any other method of the type may change the object

class Sensor {
    double base() { return 1.5; }
}

class Thermometer extends Sensor {
    private double degrees;

    // @stereotype get
    double celsius() { return degrees; }

    // @stereotype property
    double fahrenheit() { return celsius() * 9 / 5 + 32; }

    // A call to a method that reads through its own calls reads the object as well
    // @stereotype property
    double offset() { return fahrenheit() - 32; }

    // @stereotype predicate
    boolean freezing() { return this.celsius() <= 0; }

    // @stereotype incidental
    int version() { return 3; }

    // A call to a degenerate method counts like a call of a free function
    // @stereotype wrapper stateless
    int doubledVersion() { return version() * 2; }

    // @stereotype set
    void reset() { degrees = 0; }

    // A call to a set may change the object
    // @stereotype command
    void restart() { reset(); }

    // @stereotype get
    double level() { return degrees; }

    // @stereotype set
    void level(double value) { degrees = value; }

    // The call without arguments reaches the get alone, while the set of the same name takes one argument
    // @stereotype property
    double doubledLevel() { return level() * 2; }

    // A call to an inherited method counts by the stereotypes of that method
    // @stereotype wrapper stateless
    double shifted() { return base() + 1; }
}
