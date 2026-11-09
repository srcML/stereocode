// Static methods are free functions (Free-Function-Stereotypes wiki), also when defined outside the class

// @stereotype data-provider data-class small-class
class Scaler {
    int x;
public:
    static int twice(int v);
    // @stereotype property
    static int thrice(int v) { return 3 * v; }
    // @stereotype get
    int getX() const { return x; }
};
// @stereotype property
int Scaler::twice(int v) { return 2 * v; }

// @stereotype data-provider data-class small-class
class Counter {
    int n;
public:
    static int total;
    static int make();
    // @stereotype property
    static int twice(int v) { return v * 2; }
    // @stereotype get
    int get() const { return n; }
};
// @stereotype literal
int Counter::make() { return 42; }

class Tally {
    int value;
    static int total;
    static int helperDecl(int a);
    static int helperDef(int a) { return a * 2; }
public:
    static int twice(int a);
    // @stereotype property
    static int twiceIn(int a) { return a * 2; }
    // A call to a static method is a free-function call, as for viaDef
    // @stereotype property wrapper
    int viaDecl() const { return helperDecl(value); }
    // @stereotype property wrapper
    int viaDef() const { return helperDef(value); }
};
// @stereotype property
int Tally::twice(int a) { return a * 2; }
int Tally::helperDecl(int a) { return a * 2; }

// A non-static overload of a static method stays a method when defined outside the class
// @stereotype data-provider data-class small-class
class Overloads {
    int x;
public:
    static int f(int v);
    int f(double d) const;
    // @stereotype get
    int g() const { return x; }
};
// @stereotype get
int Overloads::f(double d) const { return x; }
// @stereotype property
int Overloads::f(int v) { return v * 2; }
