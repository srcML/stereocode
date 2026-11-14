// A call without an object in a class with a parent outside the archive may call an inherited method or a free function
#include <vector>

class Bag : public std::vector<int> {
    int limit;

public:
    // @stereotype predicate @defect C++ counts size() as a call of a free function, which a library may define as well
    bool none() const { return size() == 0; }

    // @stereotype predicate @defect C++ counts size() as a call of a free function, which a library may define as well
    bool full() const { return size() >= limit; }
};
