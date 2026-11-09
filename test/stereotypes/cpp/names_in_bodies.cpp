// Names in method bodies: locals, catch parameters, (*this), statics, and globals
// (Method-Stereotypes wiki, get, set, command, and incidental)

class Coordinates {
    int x;
    int y;
public:
    // @stereotype get
    int getX() { return (*this).x; }
    // @stereotype get
    int getX2() { return this->x; }
    // @stereotype set
    void setX(int v) { (*this).x = v; }
    // @stereotype command
    void setXY(int v) { (*this).x = v; (*this).y = v; }
    // @stereotype command
    void setXY2(int v) { this->x = v; this->y = v; }
};

class Shadowing {
    int x;
    int count;
public:
    // A local hides the data member only after its declaration and inside its block
    // @stereotype set
    void laterShadow() { x = 5; for (int x = 0; x < 3; x++) { } }
    // @stereotype set
    void blockShadow() { count = 1; { int count = 2; count++; } }
};

class Thrower {
    int x;
public:
    // The catch parameter is a local
    // @stereotype get
    int katch() { try { throw 1; } catch (int& e) { e = 2; } return x; }
};

// An int global and an int static member are neither object state nor objects of another class
int g;
class Tracker { int x; static int instances;
public:
  // A static data member is a global
  // @stereotype incidental
  void staticField() { instances++; }
  // @stereotype incidental
  void globalOnly() { g = 1; }
  // @stereotype incidental
  void ownTypeStatic() { Tracker::instances = 0; }
};

int g_total;
class Node {
  static int s_count;
public:
  // @stereotype incidental
  void bump() { s_count++; }
  // @stereotype incidental
  void addTotal(int v) { g_total += v; }
};
