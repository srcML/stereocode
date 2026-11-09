// Two using directives reach the two classes Canvas of other files, which C++ lookup cannot tell apart, so the definition stays a free function
namespace Gtk {}
namespace Qt {}

using namespace Gtk;
using namespace Qt;

// @stereotype literal
int Canvas::getHeight() const { return width; }
