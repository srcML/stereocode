// The object itself: a method that returns this or *this returns the object, and a write of *this changes the whole object
// (Method-Stereotypes wiki, get, set, and command)

class Node {
    int v;
    int w;
public:
    // @stereotype get
    Node* self() { return this; }
    // @stereotype get
    Node& me() { return *this; }
    // @stereotype get
    const Node& constMe() const { return (*this); }

    // A set or a command may return the object, as a method whose calls chain does
    // @stereotype set
    Node& setV(int x) { v = x; return *this; }
    // @stereotype command
    Node& setBoth(int x) { v = x; w = x; return *this; }

    // @stereotype command
    void assign(const Node& other) { *this = other; }
    // @stereotype command
    Node& reset(const Node& other) { *this = other; return *this; }
};
