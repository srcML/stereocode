// A template alias, which may name no single type, as template <typename T> using P = T*; does, and a cycle of aliases lead to
// no type of the archive, so a call through them may change the data member, and an alias of a primitive type stays a type name
// that is no primitive, so a data member of that type counts for collaborator

#include <memory>

class Engine {
    int m_rpm;
public:
    // @stereotype get
    int rpm() const { return m_rpm; }
};

template <typename T>
class Box {
public:
    // @stereotype empty
    void log() const { }
};

template <typename T> using Owner = std::unique_ptr<T>;
template <typename T> using Crate = Box<T>;
typedef Ring Loop;
typedef Loop Ring;
typedef double XYPOSITION;

class Garage {
    Owner<Engine> m_engine;
    Crate<int> m_crate;
    Loop m_loop;
public:
    // @stereotype property non-void-command collaborator
    int speed() { return m_engine->rpm() / 100; }

    // @stereotype command collaborator
    void pack() { m_crate.log(); }

    // @stereotype command collaborator
    void spin() { m_loop.turn(); }
};

class Point {
    XYPOSITION m_x;
public:
    // @stereotype property collaborator
    XYPOSITION twice() const { return m_x * 2; }
};
