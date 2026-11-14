// Return types and return expressions (Method-Stereotypes wiki, get, predicate, and property)

class Handle {
    bool open;
    int fd;
public:
    // @stereotype predicate
    explicit operator bool() const { return open && fd > 0; }
    // @stereotype predicate
    bool valid() const { return open && fd > 0; }
    // @stereotype property
    operator long() const { return fd * 2L; }
    // @stereotype property
    long twice() const { return fd * 2L; }
};

class Resource {
    void* p;
    int n;
public:
    // @stereotype predicate
    explicit operator bool() const { return p != nullptr; }
    // @stereotype property
    operator int() const { return n + 1; }
    // @stereotype predicate
    bool valid() const { return p != nullptr; }
};
class Ptr { void* p; public: explicit operator bool() const; };
// @stereotype predicate
Ptr::operator bool() const { return p != nullptr; }

class Gauge {
    int level;
public:
    // @stereotype get
    auto get() const -> int { return level; }
    // @stereotype property
    auto twice() const -> int { return level * 2; }
    // The body of a function-try-block counts
    // @stereotype set
    void reset() try { level = 0; } catch (...) { }
};

class Store {
  int m_size;
  void* m_buf;
  int m_a;
  int m_b;
public:
  // @stereotype predicate
  [[nodiscard]] bool isEmptyA() const { return m_size == 0; }
  // @stereotype predicate
  constexpr bool isEmptyB() const { return m_size == 0; }
  // @stereotype predicate
  auto isEmptyC() const -> bool { return m_size == 0; }
  // @stereotype predicate
  explicit operator bool() const { return m_size != 0; }
  // void* is no void return, while the collaborator rule counts a returned void*
  // @stereotype get non-void-command collaborator
  void* take() { m_a = 0; m_b = 0; return m_buf; }
};

class Samples {
    int arr[8];
    int x;
public:
    // @stereotype property
    int sumAt(int i) const { return arr[i] + x; }
    // @stereotype predicate
    bool isZero(int i) const { return arr[i] == 0; }
    // @stereotype get
    int at(int i) const { return arr[i]; }
};

// A return type of sign words alone, as unsigned, is an int
class Tally {
    unsigned count;
public:
    // @stereotype property
    unsigned doubled() const { return 2 * count; }
    // @stereotype property
    signed negated() const { return -count; }
    // @stereotype property
    const unsigned& next() const { return count + 1; }
};

// @stereotype property
unsigned twice(unsigned value) { return 2 * value; }

// A cast before the returned object or data member is no computation
class Gauge {
    long m_level;
    Gauge* m_next;
public:
    // @stereotype get
    int level() const { return (int)m_level; }

    // @stereotype get
    Gauge* next() const { return (Gauge*)m_next; }

    // @stereotype get
    const Gauge* self() const { return (const Gauge*)this; }

    // @stereotype property
    int twice() const { return (int)m_level * 2; }
};

// A container of Boolean values returns the container, which is no Boolean
using std::vector;

class Switches {
    vector<bool> m_states;
    vector<bool> m_none;
    int m_count;

public:
    // @stereotype property
    vector<bool> active() const { return m_count > 0 ? m_states : m_none; }
};
