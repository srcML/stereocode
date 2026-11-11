// A using directive in a namespace holds for the code after it in that namespace, also in a later block of the namespace,
// while a directive in another namespace or in a body of code holds only there
namespace Lib::Detail {
// @stereotype data-provider data-class small-class
class Widget {
    int depth;
    int size;
public:
    int getDepth() const;
    int getSize() const;
    int volume() const;
    int area() const;
};
}

namespace App {
// @stereotype empty
class Widget {
    int depth;
};
}

namespace Lib {
using namespace Detail;

// The directive names Lib::Detail from the namespace Lib around it
// @stereotype get
int Widget::getDepth() const { return depth; }
}

namespace Lib {
// @stereotype get
int Widget::getSize() const { return size; }
}

namespace Tools {
using namespace Lib::Detail;
}

// @stereotype literal
int Widget::volume() const { return depth * size; }

void useDetail() {
    using namespace Lib::Detail;
}

// @stereotype literal
int Widget::area() const { return size * size; }
