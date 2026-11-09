// Two using directives reach two classes Canvas of this file, which C++ lookup cannot tell apart, so the definition stays a free function
namespace Win {
// @stereotype empty
class Canvas {
    int width;
public:
    int getWidth() const;
};
}

namespace Mac {
// @stereotype empty
class Canvas {
    int width;
public:
    int getWidth() const;
};
}

using namespace Win;
using namespace Mac;

// @stereotype literal
int Canvas::getWidth() const { return width; }
