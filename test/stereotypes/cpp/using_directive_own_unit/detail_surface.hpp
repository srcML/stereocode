// Classes of another file that the second using directive of surface.cpp also reaches, as SurfaceImpl of PlatCocoa.h in Notepad++
namespace Lib::Detail {

// surface.cpp declares a class Surface of its own before its definitions, so none of them belongs to this class
// @stereotype data-provider data-class small-class
class Surface {
    int depth;
public:
    // @stereotype get
    int getDepth() const { return depth; }
};

// @stereotype data-provider data-class small-class
class Pen {
    int width;
public:
    int getWidth() const;
};

}
