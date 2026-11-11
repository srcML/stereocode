// A copy constructor takes the own class as its one parameter; other constructors whose parameter
// lists name the class are constructors (Method-Stereotypes wiki, Creational)

#include <vector>

class AccountStore;
class Account {
    int id;
public:
    // @stereotype constructor
    Account(const AccountStore& store) { id = store.next(); }
    // @stereotype copy-constructor
    Account(const Account& other) { id = other.id; }
};

class PointF;
class Point {
  int m_x;
public:
  // @stereotype copy-constructor
  Point(const Point& other) : m_x(other.m_x) {}
  // @stereotype constructor
  Point(const PointF& p) : m_x(0) {}
  // @stereotype constructor
  Point(int x, const Point* parent) : m_x(x) {}
  // @stereotype get
  int x() const { return m_x; }
};

enum NodeKind { Leaf, Inner };
class Node {
    NodeKind kind;
public:
    // @stereotype constructor
    Node(NodeKind k) { kind = k; }
    // @stereotype copy-constructor
    Node(const Node& other) { kind = other.kind; }
};

template <typename T> class A {
    int n;
public:
    // @stereotype copy-constructor
    A(const A<T>& other) { n = other.n; }
    A(A&& other) { n = other.n; }
    // @stereotype constructor
    A(A* parent) { n = 0; }
    // @stereotype constructor
    A(const std::vector<A>& children) { n = 0; }
    // @stereotype copy-constructor
    A(A const& other, int k = 0) { n = k; }
    // @stereotype constructor
    A(int k, const A& other) { n = k; }
};
