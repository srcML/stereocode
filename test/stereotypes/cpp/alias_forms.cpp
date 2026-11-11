// A typedef of a pointer, an alias of an alias, an alias with a template argument, a typedef in a type, and a pointer declarator
// of a typedef each lead a call on a data member to the methods of the type that they name

#include <memory>

class Engine {
    int m_rpm;
public:
    // @stereotype get
    int rpm() const { return m_rpm; }

    // @stereotype set
    void start() { m_rpm = 800; }

    // @stereotype empty
    void log() const { }

    // A method with the name of a method of the smart pointer
    // @stereotype empty
    void reset() { }
};

template <typename T>
class Box {
    int m_items;
public:
    // @stereotype get
    int items() const { return m_items; }
};

typedef Engine* EnginePtr;
using Motor = Engine;
typedef Motor Drive;
using IntBox = Box<int>;
typedef std::unique_ptr<Engine> Owner, *OwnerPtr;

class Car {
    EnginePtr m_engine;
    Drive m_drive;
    IntBox m_box;
public:
    // @stereotype property collaborator
    int speed() { return m_engine->rpm() / 100; }

    // @stereotype wrapper
    void report() { m_drive.log(); }

    // @stereotype command collaborator
    void go() { m_drive.start(); }

    // @stereotype property collaborator
    int doubled() { return m_box.items() * 2; }
};

class Truck {
    typedef Engine Diesel;
    Diesel m_diesel;
public:
    // @stereotype wrapper
    void report() { m_diesel.log(); }
};

class Garage {
    Owner m_owner;
    OwnerPtr m_slot;
public:
    // @stereotype property collaborator
    int speed() { return m_owner->rpm() / 100; }

    // The pointer declarator makes OwnerPtr a pointer to the smart pointer, which -> reaches
    // @stereotype command collaborator
    void clear() { m_slot->reset(); }
};
