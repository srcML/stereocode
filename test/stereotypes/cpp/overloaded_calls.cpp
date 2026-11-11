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
