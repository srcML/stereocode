// A method defined outside its class lies in a namespace around the class, so a using directive never attaches a definition
// in another namespace to the class that it brings in

namespace Lib {
// @stereotype empty
class Widget {
    int m_size;
public:
    int draw();
};
}

namespace Gui {
// @stereotype empty
class Widget {
    int m_depth;
public:
    int draw();
};
}

namespace App {
namespace Widget {
int draw();
}
}

using namespace Lib;

namespace App {
// The definition of the free function App::Widget::draw, since App holds no class Widget
// @stereotype literal
int Widget::draw() { return 1; }
}
