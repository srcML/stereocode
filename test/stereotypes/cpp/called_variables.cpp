// A call of a variable runs the function that the variable holds, so the call reads the variable and stays a call of a function
// (Method-Stereotypes wiki, Calls; Free-Function-Stereotypes wiki, literal and wrapper)

#include <functional>

template <typename T>
struct Functor {
    T m_functor;

    // A call of a data member reads the data member, so the method uses the state of the object
    // @stereotype wrapper
    void run() { m_functor(); }
};

struct Doubler {
    int factor;

    // @stereotype property
    int operator()(int value) const { return factor * value; }
};

struct Sequence {
    int next;

    // @stereotype property set
    int operator()() { return next++; }
};

class Button {
public:
    // @stereotype wrapper
    void click() { m_onClick(); }

    // @stereotype wrapper
    void clickThroughObject() { this->m_onClick(); }

    // A data member whose type is a class of the archive holds an object, so its call counts by the stereotypes of the call operator of that class
    // @stereotype property collaborator
    int scaled() const { return m_doubler(m_count) + 1; }

    // @stereotype property non-void-command collaborator
    int take() { return m_sequence() + 1; }

    // @stereotype wrapper
    void each(std::function<void(int)> visit) { visit(m_count); }

    // @stereotype get collaborator
    std::function<void()> handler() const { return m_onClick; }

    // @stereotype wrapper
    void fire() { std::function<void()> held = handler(); held(); }

    // A local that a lambda initializes holds code of the method itself, so its call is no call
    // @stereotype property collaborator
    int twice() const { std::function<int(int)> doubled = [](int v) { return 2 * v; }; return doubled(m_count); }

    // @stereotype set collaborator
    void bump() { std::function<void()> step = [this] { m_count++; }; step(); }

    // A data member that points to a function is a data member, which its call reads
    // @stereotype set
    void press(int code) { if (m_filter(code)) m_count++; }

    // @stereotype wrapper
    void notify(int code) const { m_filter(code); }

private:
    std::function<void()> m_onClick;
    Doubler m_doubler;
    Sequence m_sequence;
    bool (*m_filter)(int);
    int m_count;
};

// A call of a parameter reads the parameter, so the function is no literal, and changes no object of the caller
// @stereotype wrapper
void notifyAll(std::function<void(int)>& handler) { handler(1); }

// @stereotype wrapper
template <typename F> void apply(F callback) { callback(); }

// A parameter that points to a function is a parameter
// @stereotype wrapper
void run(void (*callback)()) { callback(); }

// @stereotype property wrapper
int order(int (*compare)(int, int), int left, int right) { return compare(left, right); }

// @stereotype property
int compute(int x) { auto square = [](int v) { return v * v; }; return square(x) + 1; }

std::function<void()> hook;

// A call of a global reads the global
// @stereotype wrapper
void fireHook() { hook(); }
