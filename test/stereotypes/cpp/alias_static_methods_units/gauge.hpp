// A header with an alias of its namespace and an alias of its class, which the parameter types of the static methods of the class write

namespace dials {
    typedef double Level;

    // @stereotype data-provider data-class small-class
    class Gauge {
        int reading;
    public:
        using Tick = long;
        static double clamp(Level l);
        static long advance(long t);
        // @stereotype get
        int getReading() const { return reading; }
    };
}
