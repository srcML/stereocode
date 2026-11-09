// Large and lazy structures (run with -t for structs and -u for unions): the class rules name the structure,
// as in large-struct and lazy-union (Class-Stereotypes wiki, large-class and lazy-class)
// The only class, Widget, has no methods, which gives the large-class threshold 0

class Widget {
public:
    void poke(int value);
};

// Four methods, more than the threshold, with a get, a set, a controller, and a factory in balance
// @stereotype large-struct
struct Frame {
    int width;

    // @stereotype get
    int getWidth() const { return width; }

    // @stereotype set
    void setWidth(int value) { width = value; }

    // @stereotype controller
    void paint(Widget* widget) const { widget->poke(width); }

    // @stereotype factory collaborator stateless
    Widget* make() const { return new Widget(); }
};

// @stereotype large-union
union Cell {
    int   asInt;
    float asFloat;

    // @stereotype get
    int integer() const { return asInt; }

    // @stereotype set
    void setInteger(int value) { asInt = value; }

    // @stereotype controller
    void paint(Widget* widget) const { widget->poke(asInt); }

    // @stereotype factory collaborator stateless
    Widget* make() const { return new Widget(); }
};

// Two of the four methods are degenerate, more than one third, and the others are a get and a set
// @stereotype lazy-struct
struct Slot {
    int value;

    // @stereotype get
    int get() const { return value; }

    // @stereotype set
    void set(int v) { value = v; }

    // @stereotype empty
    void reset() { }

    // @stereotype empty
    void clear() { }
};

// @stereotype lazy-union
union Bits {
    int   raw;
    float real;

    // @stereotype get
    int get() const { return raw; }

    // @stereotype set
    void set(int v) { raw = v; }

    // @stereotype empty
    void reset() { }

    // @stereotype empty
    void clear() { }
};
