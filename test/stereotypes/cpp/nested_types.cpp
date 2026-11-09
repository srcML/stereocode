// Nested named types are types of their own under their qualified names, and their methods get stereotypes,
// while local and anonymous types stay out (Class-Stereotypes wiki)

// @stereotype data-provider lazy-class small-class
class Outer {
    int x;
public:
    // @stereotype get
    int getX() const { return x; }
    // @stereotype data-class small-class
    class Inner { int v; public:
        // @stereotype get
        int get() const { return v; }
        void set(int value);
    };
    // @stereotype incidental
    void run() const { struct Local { int w; int get() const { return w; } }; }
};

// A method of a nested type, defined outside the type under its qualified name
// @stereotype set
void Outer::Inner::set(int value) { v = value; }

// A parent named inside the outer type is a nested type of the outer type before a type of the namespace
class Base { int m_other; };
class Tree {
    class Base { protected: int m_count; };
    class Counter : public Base { public:
        // @stereotype set
        void reset() { m_count = 0; }
    };
    // A type nested in a nested type
    struct Node { struct Leaf { int m_value; public:
        // @stereotype get
        int value() const { return m_value; } }; };
};

// A type nested in a class template
template<typename T>
class Box {
    class Iterator { int m_index; public:
        // @stereotype get
        int index() const { return m_index; }
    };
};
