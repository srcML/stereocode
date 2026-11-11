// Class signatures count methods, once each (Class-Stereotypes wiki, rules for class stereotype identification)

// bump is get and set and twice is property, so the class has a method other than get and set
// @stereotype small-class
class Counter {
  int m_n;
public:
  // @stereotype get set
  int bump() { m_n = 5; return m_n; }
  // @stereotype property
  int twice() const { return m_n * 2; }
};

// Entity and minimal-entity allow at most two collaborational methods per other method; the
// dissertation names Range an entity and Point a minimal entity
// @stereotype entity
class Range {
  double m_min;
  double m_max;
public:
  double low() const { return m_min; }
  double length() const { return m_max - m_min; }
  void setLow(double v) { m_min = v; }
  void setRange(double a, double b) { m_min = a; m_max = b; }
};

// @stereotype minimal-entity
class Point {
  int m_x;
  int m_y;
public:
  int x() const { return m_x; }
  void setX(int v) { m_x = v; }
  void moveBy(int dx, int dy) { m_x += dx; m_y += dy; }
};

class Scale;

// Three collaborational methods per other method are too many for an entity
// @stereotype boundary
class Dial {
  int m_value;
  int m_turns;
public:
  // @stereotype property collaborator
  int scaled(const Scale& scale) const { return m_value * 2; }
  // @stereotype command collaborator
  void turn(const Scale& scale) { m_value *= 2; m_turns++; }
  // @stereotype command collaborator
  void reset(const Scale& scale) { m_value = 0; m_turns = 0; }
  // @stereotype get
  int turns() const { return m_turns; }
};

// One build compiles one branch of #if and #else, so boost counts once
#define API __attribute__((visibility("default")))
// @stereotype data-class small-class
class API Engine {
    int rpm;
public:
    int speed() const { return rpm; }
#if defined(FAST)
    void boost() { rpm = 9000; }
#else
    void boost() { rpm = 5000; }
#endif
};

// Overloads that differ only in a const of the type of a parameter are two methods
// @stereotype data-provider data-class
class Index {
    int m_count;
public:
    // @stereotype get
    int count(int* first) const { return m_count; }
    // @stereotype get
    int count(const int* first) const { return m_count; }
    // @stereotype get
    int size() const { return m_count; }
};
