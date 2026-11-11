// A call on a data member whose type is a type of the archive counts by the stereotypes of the called method of that type:
// a call to an accessor reads the data member, a call to a degenerate method counts like a call of a free function,
// and a call to any other method may change the data member

#include <cstdio>
#include <vector>

class Engine {
    int m_rpm;
public:
    // @stereotype get
    int rpm() const { return m_rpm; }

    // @stereotype set
    void start() { m_rpm = 800; }

    // @stereotype wrapper stateless
    void log() const { std::puts("engine"); }

    // @stereotype get
    int& throttle() { return m_rpm; }
};

class Car {
    Engine m_engine;
    Engine* m_spare;
    std::vector<int> m_trips;
public:
    // @stereotype property collaborator
    int speed() { return m_engine.rpm() / 100; }

    // @stereotype property collaborator
    int spareSpeed() { return m_spare->rpm() / 100; }

    // @stereotype command collaborator
    void go() { m_engine.start(); }

    // @stereotype wrapper
    void report() { m_engine.log(); }

    // A write through the result of an accessor changes the data member
    // @stereotype command collaborator
    void idle() { m_engine.throttle() = 0; }

    // A call on a data member whose type is no type of the archive may change the data member
    // @stereotype command
    void trip(int length) { m_trips.push_back(length); }
};
