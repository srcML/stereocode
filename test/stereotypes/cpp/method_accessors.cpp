// Accessors (Method-Stereotypes wiki, Structural Accessor): get, predicate, property, and void-accessor

class Thermometer {
    double m_celsius;
    bool   m_calibrated;
public:
    // @stereotype get
    double celsius() const { return m_celsius; }

    // @stereotype get
    bool isCalibrated() const { return m_calibrated; }

    // @stereotype predicate
    bool isFreezing() const { return m_celsius <= 0.0; }

    // @stereotype property
    double fahrenheit() const { return m_celsius * 9.0 / 5.0 + 32.0; }

    // @stereotype void-accessor
    void readCelsius(double& celsius) const { celsius = m_celsius; }
};
