// A call to an inherited method counts by the stereotypes of that method, also through the name of the parent

class Sensor {
protected:
    double m_base;
public:
    // @stereotype get
    double base() const { return m_base; }

    // @stereotype incidental
    int version() const { return 2; }

    // @stereotype set
    void calibrate(double value) { m_base = value; }
};

class Probe : public Sensor {
    double m_reading;
public:
    // @stereotype property
    double offset() { return m_reading - base(); }

    // @stereotype property
    double scaled() { return Sensor::base() * 2; }

    // @stereotype property
    double shifted() { return this->base() + 1; }

    // @stereotype wrapper stateless
    int doubledVersion() { return version() * 2; }

    // A call to an inherited set may change the object
    // @stereotype command
    void reset() { calibrate(0); }
};

class Counter {
protected:
    int m_count;
    int m_reads;
public:
    // @stereotype get set
    int count() { m_reads++; return m_count; }
};

class QuietCounter : public Counter {
public:
    // A method with the signature of a method of the parent replaces that method in the calls of the child
    // @stereotype get
    int count() { return m_count; }

    // @stereotype property
    int doubled() { return count() * 2; }
};
