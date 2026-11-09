// Returns, writes, and locals inside lambdas belong to the lambda, not to the enclosing function
// (Method-Stereotypes wiki, get and set; Free-Function-Stereotypes wiki, global-command and property)

#include <algorithm>
#include <functional>

// The returned parameter n makes a property
// @stereotype property wrapper
int zeroAll(int* p, int n) {
  std::for_each(p, p + n, [](int& x) { x = 0; });
  return n;
}

class Buffer {
  int m_size;
public:
  // @stereotype wrapper stateless
  void zero(int* p, int n) const { std::for_each(p, p + n, [](int& x) { x = 0; }); }
  void visit() const { auto f = [this]() { return m_size; }; }
};

// @stereotype small-class
class Timer {
    int ticks;
public:
    // A void method is no get
    // @stereotype controller
    void schedule(Scheduler& s) { s.post([this]() { return ticks; }); }
    // @stereotype set
    void reset() { ticks = 0; }
};

// The local total of the lambda is no data member, and the static total is no object state
static int total = 0;
class Calc {
    std::function<int(int)> op = [](int x) { int total = x * 2; return total; };
public:
    // @stereotype incidental
    int peek() const { return total; }
};
