// A using directive reaches a class template and its specialization for the written template arguments
namespace Lib {
// @stereotype data-class small-class
template <typename T>
class Box {
    int count;
public:
    int size() const;
    void clear();
};

// @stereotype data-provider data-class small-class
template <>
class Box<int> {
    int count;
public:
    int size() const;
};
}

namespace Other {
// @stereotype empty
template <typename T>
class Box {
    int count;
public:
    int size() const;
};
}

using namespace Lib;

// @stereotype get
template <typename T>
int Box<T>::size() const { return count; }

// @stereotype set
template <typename T>
void Box<T>::clear() { count = 0; }

// @stereotype get
int Box<int>::size() const { return count; }
