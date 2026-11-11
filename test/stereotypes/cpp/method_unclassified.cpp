// Unclassified methods (Method-Stereotypes wiki): a method that fits no rule, such as a method that reads a data member
// without returning it, changing it, or making a call other than an ignored call

class Counter {
    int m_count;
public:
    // assert is an ignored call
    // @stereotype unclassified
    void check() const { assert(m_count >= 0); }

    // @stereotype unclassified
    void touch() const { int copy = m_count; }
};
