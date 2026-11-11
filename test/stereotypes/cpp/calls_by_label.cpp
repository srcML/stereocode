// A call to a method of the own type counts by the stereotypes of that method: a call to an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object and counts like a call of a free function,
// and a call to any other method may change the object

#include <cstdio>

// The incidental bar is no accessor, and test, which calls bar, uses no state of the object
class foo {
public:
    // @stereotype incidental
    int bar() { return 4; }

    // @stereotype wrapper stateless
    bool test() { return bar() + 5; }
};

class Cell {
    int m_value;
public:
    // @stereotype get
    int& value() { return m_value; }

    // @stereotype empty
    void check() { }

    // @stereotype wrapper stateless
    void print() const { std::puts("cell"); }

    // @stereotype property wrapper
    int checked() { check(); return m_value * 2; }

    // @stereotype wrapper stateless
    void refresh() { check(); }

    // A write through the result of a get changes the object
    // @stereotype command
    void clear() { value() = 0; }

    // @stereotype property wrapper
    int printed() { print(); return value() + 1; }
};
