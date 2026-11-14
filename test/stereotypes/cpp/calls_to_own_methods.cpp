// Calls to the methods of the own type: a call to a method labeled as an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object, and a call to any other method of the type may change the object

class Thermometer {
    double m_degrees;
public:
    // @stereotype get
    double celsius() { return m_degrees; }

    // @stereotype property
    double fahrenheit() { return celsius() * 9 / 5 + 32; }

    // @stereotype predicate
    bool freezing() { return this->celsius() <= 0; }

    // @stereotype property
    double kelvin() { return (*this).celsius() + 273.15; }

    // @stereotype set
    void reset() { m_degrees = 0; }

    // A call to a set may change the object
    // @stereotype command
    void restart() { reset(); }

    double rankine();
};

// A call to a method that reads through its own calls reads the object as well
// @stereotype property
double Thermometer::rankine() { return fahrenheit() + 459.67; }

// A call that reaches the calling method itself counts by the stereotypes that the method has without the call
class Limit {
    int m_limit;
public:
    // @stereotype predicate
    bool accepts(int value) { return value < m_limit; }

    // The call with one argument reaches the two overloads, of which the other one only reads
    // @stereotype predicate
    bool accepts(long value) { return accepts(static_cast<int>(value)); }

    // @stereotype property
    int next(int level) { return level > m_limit ? level : next(level + 1); }

    // A call of a method that changes the object to itself may change the object
    // @stereotype command
    void drain(int steps) { if (steps > 0) { m_limit--; drain(steps - 1); } }
};
