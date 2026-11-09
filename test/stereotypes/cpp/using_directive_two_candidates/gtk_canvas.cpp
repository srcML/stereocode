// One using directive reaches one of the two classes Canvas, so the definition belongs to that class
namespace Gtk {}

using namespace Gtk;

// @stereotype get
int Canvas::getWidth() const { return width; }
