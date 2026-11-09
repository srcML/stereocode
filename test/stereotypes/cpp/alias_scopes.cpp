// An alias hides a type of its name that the code reaches only through a farther scope or a using directive, from its declaration on
// and outside the bodies of code, while a type of a nearer scope hides the alias, as the name lookup of C++ does

class Engine {
    int m_rpm;
public:
    // @stereotype set
    void start() { m_rpm = 800; }

    // @stereotype empty
    void log() const { }
};

namespace parts {
    class Motor {
        int m_load;
    public:
        // @stereotype set
        void log() { m_load = 0; }
    };

    class Gauge {
        int m_level;
    public:
        // @stereotype set
        void log() { m_level = 0; }
    };
}

namespace fleet {
    // The alias of the namespace of the van hides parts::Motor
    typedef Engine Motor;

    class Van {
        Motor m_motor;
    public:
        // @stereotype wrapper
        void report() { m_motor.log(); }
    };
}

typedef Engine Pump;

namespace plant {
    // The class of the namespace of the station hides the alias of the global namespace
    class Pump {
        int m_flow;
    public:
        // @stereotype set
        void log() { m_flow = 0; }
    };

    class Station {
        Pump m_pump;
    public:
        // @stereotype command collaborator
        void report() { m_pump.log(); }
    };
}

class Winch {
    int m_load;
public:
    // @stereotype set
    void log() { m_load = 0; }
};

namespace dock {
    class Barge {
        Winch m_winch;
    public:
        // The alias dock::Winch comes after the barge, so the data member has the class Winch of the global namespace
        // @stereotype command collaborator
        void report() { m_winch.log(); }
    };

    typedef Engine Winch;
}

namespace garage {
    // An alias that takes the name of the class that it names, which its target thus never names
    using Engine = ::Engine;

    class Car {
        Engine m_engine;
    public:
        // @stereotype wrapper
        void report() { m_engine.log(); }
    };
}

namespace yard {
    typedef Engine Engine;

    class Crane {
        Engine m_engine;
    public:
        // @stereotype wrapper
        void report() { m_engine.log(); }
    };
}

void tune() {
    using Motor = Engine;
    typedef Engine Gauge;
}

class Depot {
    Motor m_motor;
    Gauge m_gauge;
public:
    // An alias in a body of code holds only in that body, so the data members have the classes of the namespace parts
    // @stereotype command collaborator
    void report() { m_motor.log(); }

    // @stereotype command collaborator
    void check() { m_gauge.log(); }
};
