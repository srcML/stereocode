// The alias of the type of a data member counts alike when a later file of the archive declares the alias, as b_engine.hpp does

class Car {
    Motor m_motor;
public:
    // @stereotype property collaborator
    int speed() { return m_motor.rpm() / 100; }
};
