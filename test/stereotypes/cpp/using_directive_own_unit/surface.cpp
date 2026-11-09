// The methods defined after the namespace closes belong to the class of this file that a using directive reaches,
// although the second directive reaches a class of the same name in another file, as SurfaceImpl of PlatGTK.cxx in Notepad++
namespace Lib::Detail {}

using namespace Lib;
using namespace Lib::Detail;

namespace Lib {
// @stereotype entity
class Surface {
    int width;
    int height;
public:
    int getWidth() const;
    void setWidth(int w);
    int area() const;
    void resize(int w, int h);
};
}

// @stereotype get
int Surface::getWidth() const { return width; }

// @stereotype set
void Surface::setWidth(int w) { width = w; }

// @stereotype property
int Surface::area() const { return width * height; }

// @stereotype command
void Surface::resize(int w, int h) {
    width = w;
    height = h;
}

// The class Pen of this file comes after the definition, so the definition belongs to the class Pen of the other file
// @stereotype get
int Pen::getWidth() const { return width; }

namespace Lib {
// @stereotype empty
class Pen {
    int width;
};
}
