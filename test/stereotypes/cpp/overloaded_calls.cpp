// Calls to overloaded methods of the own type: a call reaches the overloads of its name that take as many arguments as the call passes

#include <string>
using namespace std;

class Gauge {
    double m_level;
    int m_offset;
public:
    // @stereotype get
    double level() { return m_level; }

    // @stereotype set
    void level(double value) { m_level = value; }

    // The call without arguments reaches the get alone, while the set takes one argument
    // @stereotype property
    double doubled() { return level() * 2; }

    // @stereotype property
    int scale(int unit) { return m_offset * unit; }

    // @stereotype set
    void scale(double factor) { m_level = m_level * factor; }

    // Both overloads take one argument, and the set may change the object
    // @stereotype property non-void-command
    int scaled() { return scale(2) + 1; }

    int offset(int unit = 1);

    // @stereotype set
    void offset(int low, int high) { m_offset = high - low; }

    // The default argument of the declaration lets the call without arguments reach the definition outside the class
    // @stereotype property
    int shifted() { return offset() + 1; }

    // @stereotype get
    int total(int first, ...) { return m_offset; }

    // @stereotype set
    void total(int first, int second) { m_offset = first + second; }

    // A call with three arguments reaches the variadic overload alone
    // @stereotype property
    int summed() { return total(1, 2, 3) + 1; }
};

// @stereotype property
int Gauge::offset(int unit) { return m_offset * unit; }

class Journal {
    int m_level;
public:
    void write(const std::string& text, int count = 1);

    // @stereotype set
    void write(int level, int first, int second) { m_level = level + first + second; }

    // The default argument of the declaration holds although the definition spells the type of the text without std::
    // The call thus reaches the empty overload alone, which counts as a call of a free function
    // @stereotype get wrapper
    int written() { write("entry"); return m_level; }
};

// @stereotype empty
void Journal::write(const string& text, int count) { }

// A call whose number of arguments only a static method of its name takes is a call of that static method, which is a free function
class Label {
    const char* m_key;
public:
    static bool isData(const char* key) { return key != nullptr; }

    // @stereotype predicate wrapper
    bool isData() { return isData(m_key); }

    static int width(int margin, int border);

    // @stereotype set
    void width(int value) { m_key = value > 0 ? "w" : ""; }

    // The call with one argument reaches the method
    // @stereotype command
    void widen() { width(4); }

    // The static method is defined outside the class
    // @stereotype wrapper stateless
    int padded() { return width(1, 2); }
};

int Label::width(int margin, int border) { return margin + border; }

// A call whose number of arguments only a method without a definition takes reaches no method with stereotypes and may thus change the object
class Timer {
public:
    virtual int schedule(int task, long delay, int unit) = 0;

    // The call names the pure virtual method, whose definitions lie in other classes, never the calling method itself
    // @stereotype non-void-command
    int schedule(int task) { return schedule(task, 0L, 1); }

    static int width(int margin, int border) { return margin + border; }

    virtual int width(const char* text, const char* font) = 0;

    // The pure virtual method takes two arguments as the static method does
    // @stereotype non-void-command
    int padded() { return width("a", "b"); }

    virtual int delay(int task) = 0;

    // The call names the pure virtual method, which takes another type than the calling method
    // @stereotype non-void-command
    int delay(long task) { return delay(static_cast<int>(task)); }
};

class Alarm : public Timer {
    int m_limit;
public:
    using Timer::schedule;

    // The pure virtual method of the parent takes other types than the calling method
    // @stereotype non-void-command
    int schedule(const char* task, long delay, int unit) { return schedule(0, delay, unit); }

    // The method defines the pure virtual method of the parent, and its call is a call to itself
    // @stereotype property
    int width(const char* text, const char* font) override { return text == font ? m_limit : width(font, font); }
};

// Where no method of the name takes the arguments of a call, a parent outside the archive may define the called method
class Names : public Roster {
public:
    using Roster::add;
    using Roster::append;
    using Roster::size;

    // @stereotype non-void-command
    bool add(const char* name) { add(0, name); return true; }

    // The call without arguments names the query method size of the parent, which reads the object
    // @stereotype property
    int size(int scale) { return size() * scale; }

    // The parent may define the called method as well, which takes one argument as the calling method does
    // @stereotype command
    void append(long count) { append(static_cast<int>(count)); }
};
