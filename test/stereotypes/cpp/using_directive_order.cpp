// A using directive holds only for the code after it, so a method defined before the directive stays a free function
// when two classes of its name exist
namespace Shapes {
// @stereotype data-provider data-class small-class
class Circle {
    int r;
public:
    int radius() const;
    int diameter() const;
};
}

namespace Sketch {
// @stereotype empty
class Circle {
    int r;
public:
    int radius() const;
};
}

// @stereotype literal
int Circle::diameter() const { return 2 * r; }

using namespace Shapes;

// @stereotype get
int Circle::radius() const { return r; }
