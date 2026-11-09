// Data members of known and unknown parents (Method-Stereotypes wiki, get, set, and incidental)

class Shape { protected: int m_sides; };
class Square : public Shape {
public:
    // @stereotype get
    int sides() const { return m_sides; }
    // A parent that qualifies a name, as Shape does, names a data member of the object
    // @stereotype set
    void setSides(int sides) { Shape::m_sides = sides; }
};

// An unknown parent may declare the written data member, also an object whose member the method writes
class Panel : public ui::Widget {
public:
    // @stereotype set
    void setLevel(int level) { m_level = level; }
    // @stereotype set
    void hideView() { m_view.visible = false; }
    // A class that qualifies a name with :: names no data member of the object
    // @stereotype collaborator incidental
    void countPanels() { Registry::count = 1; }
};

// An unknown parent may declare a written name, but not a static data member of the type or of a known parent
class Counted { protected: static int s_total; int m_size; };
class Gauge : public Counted, public ui::Widget {
    static int s_instances;
public:
    // @stereotype incidental
    void count() { s_instances++; }
    // @stereotype incidental
    void total() { Counted::s_total++; }
    // @stereotype incidental
    void totalPlain() { s_total = 0; }
    // @stereotype set
    void grow() { Counted::m_size++; }
    // @stereotype set
    void level(int v) { m_level = v; }
};

// The nested type Inner of a template declares no counter, so counter is the global
int counter = 0;
template<typename T>
struct Holder {
    int counter;
    struct Inner { };
};
struct Leaf : public Holder<int>::Inner {
    // @stereotype incidental
    int peek() const { return counter; }
};
struct Leaf2 : public Inner2 {
    // @stereotype incidental
    int peek() const { return counter; }
};
