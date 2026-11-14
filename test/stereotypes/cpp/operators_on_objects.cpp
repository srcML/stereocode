// An operator applied to an object of a class calls the operator function of that class
// (Method-Stereotypes wiki, Calls; Free-Function-Stereotypes wiki, command)

#include <iostream>
#include <sstream>

class Buffer {
    int values[8];
    int count;
public:
    // @stereotype command
    Buffer& operator<<(int value) { values[count] = value; count++; return *this; }

    // An operator after the object itself calls the operator of the own type
    // @stereotype command
    Buffer& operator<<(bool flag) { *this << (flag ? 1 : 0); return *this; }

    // @stereotype command
    Buffer& append(int first, int second) { (*this) << first << second; return *this; }

    // @stereotype predicate
    bool operator==(const Buffer& other) const { return count == other.count; }

    // The operator of an accessor reads the object
    // @stereotype predicate
    bool operator!=(const Buffer& other) const { return !(*this == other); }
};

struct Base {
    int a;
    // @stereotype set
    Base& operator=(const Base& other) { a = other.a; return *this; }
};

// A statement that starts with the name of an operator function is a call, which srcML marks as a declaration
struct Derived : Base {
    int b;
    // @stereotype command
    Derived& operator=(const Derived& other) { Base::operator=(other); b = other.b; return *this; }

    // @stereotype command
    Derived& assign(const Derived& other) { operator=(other); return *this; }

    // @stereotype command
    Derived& assignThroughObject(const Derived& other) { this->operator=(other); return *this; }
};

// The name in the brackets of such a statement is an argument of the call
// @stereotype wrapper
void release(void* block) { ::operator delete(block); }

struct Point { int x; int y; };

// A stream operator is a call on the stream, which may change the stream
// @stereotype command
void log(std::ostream& out, int value) { out << value; }

// @stereotype command
void read(std::istream& in, Point& point) { in >> point.x >> point.y; }

// @stereotype property command
std::ostream& operator<<(std::ostream& out, const Point& point) { return out << point.x << point.y; }

// An operator of a class of the archive applied to a parameter is a call on the parameter
// @stereotype command
void fill(Buffer& buffer) { buffer << 1 << 2; }

// A shift of a number is no call
// @stereotype property
int high(int word) { return word >> 16; }

// @stereotype property
unsigned mask(unsigned bits, int width) { return bits << width; }

class Reading {
    int value;
    int scale;
    std::ostringstream text;
public:
    // The variable that a stream operator extracts into is written
    // @stereotype set collaborator
    void load(std::istream& in) { in >> value; }

    // @stereotype command collaborator
    void loadAll(std::istream& in) { in >> value >> scale; }

    // A stream operator on a data member is a call on the data member
    // @stereotype command collaborator
    void print() { text << value; }

    // @stereotype property
    int half() const { return value >> 1; }

    // @stereotype controller
    void show(std::ostream& out) const { out << value; }
};

struct Counters { unsigned count[2]; };

class Digest {
    Counters context;
public:
    // A shift of a number inside a data member is no call on the data member
    // @stereotype property collaborator
    unsigned index() { return (context.count[0] >> 3) & 0x3F; }
};
