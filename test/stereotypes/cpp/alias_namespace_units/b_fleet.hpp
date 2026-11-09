// A header with an alias of the namespace fleet, which holds in the files that include the header, as a_van.cpp may

class Engine {
    int m_rpm;
public:
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
}

namespace fleet {
    typedef Engine Motor;
}
