// Data members: anonymous member types, own and container types, and locals that are no data members
// (Method-Stereotypes wiki, get, property, and collaborator)

#include <array>
#include <string>
#include <vector>

struct Packet {
    struct { int a; int b; } header;
    int len;
    // header is a data member
    // @stereotype property
    int first() const { return header.a; }
    // @stereotype get
    int size() const { return len; }
};

// The own generic type and containers of primitives are no objects of other classes
template<class T>
class Node {
    Node<T>* next;
    Node* prev;
    std::array<int, 3> dims;
    std::wstring label;
    std::string name;
public:
    // @stereotype get
    Node<T>* getNext() const { return next; }
    // @stereotype get
    Node* getPrev() const { return prev; }
    // @stereotype get
    int dim0() const { return dims[0]; }
    // @stereotype get
    std::wstring getLabel() const { return label; }
    // @stereotype get
    std::string getName() const { return name; }
};

class Dims {
    std::array<int, 3> fixed;
    std::vector<int> dynamic;
public:
    // @stereotype get
    int fixed0() const { return fixed[0]; }
    // @stereotype get
    int dynamic0() const { return dynamic[0]; }
};

// The methods of an anonymous type are stereotyped like those of a named type
struct {
    int x;
    // @stereotype get @defect type-model-B23
    int get() const { return x; }
} counter;
class Named {
    int y;
public:
    // @stereotype get
    int get() const { return y; }
};

// The local limit of the constructor is no data member, so capacity reads the global
int limit = 8;
class Buffer {
    int size;
public:
    // @stereotype constructor
    Buffer() { int limit = 4; size = limit; }
    // @stereotype incidental
    int capacity() const { return limit; }
};
