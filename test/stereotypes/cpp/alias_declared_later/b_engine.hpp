// The alias of the type of a data member counts alike when a later file of the archive declares the alias, as for a_car.cpp

class Engine {
    int m_rpm;
public:
    // @stereotype get
    int rpm() const { return m_rpm; }
};

typedef Engine Motor;
