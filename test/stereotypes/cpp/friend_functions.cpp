// Friend functions are free functions (Free-Function-Stereotypes wiki), also when defined in the class body

// @stereotype data-provider data-class small-class
class A {
    int x;
public:
    // @stereotype property
    friend int peek(const A& a) { return a.x; }
    // @stereotype get
    int getX() const { return x; }
};

// @stereotype data-class small-class
class Point {
    int x;
    int y;
public:
    // @stereotype get
    int getX() const { return x; }
    // @stereotype set
    void setX(int v) { x = v; }
    // @stereotype predicate
    friend bool operator==(const Point& a, const Point& b) { return a.x == b.x && a.y == b.y; }
    // @stereotype property
    friend int area(const Point& p) { return p.x * p.y; }
};

// @stereotype empty
class Pixel {
    int x;
    int y;
public:
    // @stereotype predicate
    friend bool operator==(const Pixel& a, const Pixel& b) { return a.x == b.x && a.y == b.y; }
    // In a friend function defined in a class, srcML 1.2 leaves a.x outside the expression of a.x = b.x, which hides the write
    // @stereotype command @defect srcML leaves a.x outside the expression
    friend void swapX(Pixel& a, Pixel& b) { int t = a.x; a.x = b.x; b.x = t; }
    friend int norm1(const Pixel& p);
};
// @stereotype property
int norm1(const Pixel& p) { return p.x + p.y; }

class Logged;
void log(const Logged& a);
void report(const Logged& a);
class Logged {
    int x;
    friend void log(const Logged& a);
public:
    // The friend declaration names a free function, not a method of the class
    // @stereotype get wrapper
    int viaFriend() const { log(*this); return x; }
    // @stereotype get wrapper
    int viaFree() const { report(*this); return x; }
};

// One method remains once the friend and the static methods are free functions
// @stereotype data-provider data-class small-class
class Dot {
  int m_x;
  static int s_made;
public:
  // @stereotype get
  int x() const { return m_x; }
  // @stereotype predicate
  friend bool operator==(const Dot& a, const Dot& b) { return a.m_x == b.m_x; }
  // A static method that reads a static data member uses a global, so it is no literal
  // @stereotype unclassified
  static int madeInClass() { return s_made; }
  static int madeOutOfClass();
};

// @stereotype unclassified
int Dot::madeOutOfClass() { return s_made; }
