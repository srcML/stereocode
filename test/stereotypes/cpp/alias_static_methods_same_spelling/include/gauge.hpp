// Headers with aliases of the global namespace, which the definitions of the static methods of their classes write as the declarations do

typedef int Count;
typedef int Level;

// @stereotype data-provider data-class small-class
class Gauge {
    int reading;
public:
    static int twice(Count c);
    // @stereotype get
    int getReading() const { return reading; }
};

// @stereotype data-provider data-class small-class
class Dial {
    int position;
public:
    static int turn(Level l);
    // @stereotype get
    int getPosition() const { return position; }
};
