// Calls to methods of the own class through this, (*this), or a class name are internal, and qualified
// free functions are free functions (Method-Stereotypes wiki, command and wrapper)

#include <cstdio>

class Base { public: virtual void draw() {} };
class A : public Base {
    int x;
public:
    // @stereotype set
    void g() { x = 1; }
    // @stereotype command
    void callPlain() { g(); }
    // @stereotype command
    void callThis() { this->g(); }
    // @stereotype command
    void callStar() { (*this).g(); }
    // @stereotype command
    void callQualifiedOwn() { A::g(); }
    // The empty draw of Base is a degenerate method, whose call counts like a call of a free function
    // @stereotype wrapper stateless
    void draw() { Base::draw(); }
    // @stereotype wrapper stateless
    void plainFree() { puts("x"); }
    // @stereotype wrapper stateless
    void qualifiedFree() { std::puts("x"); }
    // @stereotype wrapper stateless
    void globalFree() { ::puts("x"); }
};

class Engine {
  int m_rpm;
public:
  // @stereotype set
  void refresh() { m_rpm = 0; }
  // @stereotype command
  void viaThis() { this->refresh(); }
  // @stereotype command
  void direct() { refresh(); }
  // The body of a function-try-block counts
  // @stereotype set
  void guarded() try { m_rpm = 1; } catch (...) { }
};
