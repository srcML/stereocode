// The definition of a static method outside its class matches its declaration also where one of the two writes a parameter type through an alias
// that only renames the type, a typedef or an alias declaration of the namespace or of the class, also through other aliases, so the definition
// stays a free function (Free-Function-Stereotypes wiki)

typedef int Count;
typedef Count Total;
using Ratio = double;

// A typedef in the declaration, an alias declaration in the definition, and an alias of an alias with a specifier and a modifier
// @stereotype data-provider data-class small-class
class Meter {
    int x;
public:
    static int twice(Count v);
    static double half(double r);
    static int thrice(const Total& t);
    // @stereotype get
    int get() const { return x; }
};
// @stereotype property
int Meter::twice(int v) { return 2 * v; }
// @stereotype property
double Meter::half(Ratio r) { return r / 2; }
// @stereotype property
int Meter::thrice(const int& t) { return 3 * t; }

// An alias of the class, which the definition writes with or without the name of the class
// @stereotype data-provider data-class small-class
class Buffer {
    int size;
public:
    typedef long Offset;
    static long shift(Offset o);
    static long skip(long o);
    // @stereotype get
    int getSize() const { return size; }
};
// @stereotype property
long Buffer::shift(long o) { return o + 1; }
// @stereotype property
long Buffer::skip(Buffer::Offset o) { return o + 2; }

// An alias of the namespace of the class, which the definition outside the namespace writes with the name of the namespace
namespace units {
    using Meters = double;

    // @stereotype data-provider data-class small-class
    class Distance {
        double value;
    public:
        static double scale(Meters m, double factor);
        // @stereotype get
        double get() const { return value; }
    };
}
// @stereotype property
double units::Distance::scale(double m, units::Meters factor) { return m * factor; }
