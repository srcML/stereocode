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

// A member alias of a parent names the object itself, as the name of the parent does
class Gauge : public Sensor {
    using Base = Sensor;
    typedef Sensor Parent;
    using Reading = double;
public:
    // @stereotype property
    double tripled() { return Base::base() * 3; }

    // @stereotype command
    void clear() { Parent::calibrate(0); }

    // An alias of another type names no parent
    // @stereotype wrapper stateless
    double limit() { return Reading::max(); }
};

class FineGauge : public Gauge {
public:
    // A child reaches the member aliases of its parents
    // @stereotype property
    double halved() { return Base::base() / 2; }
};

class Pile : public std::vector<int> {
    using Container = std::vector<int>;
public:
    // A method of a parent outside the archive may change the object, while a query method reads the object
    // @stereotype command
    void shrink() { Container::resize(Container::size() - 1); }

    // @stereotype property
    unsigned held() { return Container::size(); }

    // A const method calls only the const methods of a parent, which read the object
    // @stereotype property
    int top() const { return this->back(); }

    // @stereotype property
    int bottom() const { return Container::front(); }

    // @stereotype non-void-command
    int last() { return this->back(); }

    // An operator function of a parent is a method of the parent
    // @stereotype property
    int at(unsigned place) const { return Container::operator[](place); }

    // @stereotype command
    void copy(const Pile& other) { Container::operator=(other); }
};
