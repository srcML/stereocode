// The names that a class writes resolve first among its nested types and member aliases, which hide the types and aliases
// of the namespaces around the class, as the name lookup of C++ does

class Engine {
    int m_rpm;
public:
    // @stereotype set
    void log() { m_rpm = 0; }
};

class Pump {
public:
    // @stereotype empty
    void log() const { }
};

class Motor {
    int m_load;
public:
    // @stereotype set
    void log() { m_load = 0; }
};

typedef Engine Timer;

class Clock {
    struct Timer {
        int m_ticks;

        // @stereotype empty
        void log() const { }
    };
    Timer m_timer;
public:
    // The nested struct Clock::Timer hides the typedef Timer of the global namespace
    // @stereotype wrapper
    void report() { m_timer.log(); }
};

class Truck {
    typedef Pump Motor;
    typedef Pump Diesel;
    Motor m_motor;
    Diesel m_diesel;
public:
    // The member typedef Truck::Motor hides the class Motor of the global namespace
    // @stereotype wrapper
    void report() { m_motor.log(); }

    // @stereotype wrapper
    void drain() { m_diesel.log(); }
};

class Bus {
    typedef Engine Diesel;
    Diesel m_diesel;
public:
    // The member typedef Bus::Diesel names Engine, unlike the member typedef of the truck
    // @stereotype command collaborator
    void drain() { m_diesel.log(); }
};

namespace store {
    class Arena {
        typedef Pump Allocator;
        Allocator m_allocator;
    public:
        // @stereotype wrapper
        void clear() { m_allocator.log(); }
    };
}

template <class Allocator>
class Vector {
    Allocator m_allocator;
public:
    // The member typedef store::Arena::Allocator holds outside the arena only where a name writes the arena,
    // so the template parameter names no type of the archive
    // @stereotype command collaborator
    void clear() { m_allocator.log(); }
};

using namespace store;

class Depot {
    Arena::Allocator m_allocator;
public:
    // @stereotype wrapper
    void clear() { m_allocator.log(); }
};
