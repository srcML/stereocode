// Method-Stereotypes wiki, Collaborational, C++ examples (HippoDraw and OpenGL), each in a minimal class

#include <string>
using std::string;

class PlotterBase;
class DataRep;
namespace Axes { enum Type { X, Y, Z }; }

class DisplayController {
public:
  void fixLogIfBinned(const PlotterBase* plotter, DataRep* rep) const;
  void setBinner(const PlotterBase* plotter, DataRep* rep, Axes::Type axis) const;
};

// @stereotype collaborator
void DisplayController::fixLogIfBinned ( const PlotterBase* plotter, DataRep* rep ) const {
  setBinner ( plotter, rep, Axes::X );
  setBinner ( plotter, rep, Axes::Y );
}

class XmlElement;
class BinnerAxis;
class Range;

class BinnerAxisXML {
  string m_type;
  string m_high;
  string m_low;
  string m_width;
public:
  void setAttributes(XmlElement* tag, const BinnerAxis& binner);
};

// @stereotype controller
void BinnerAxisXML::setAttributes(XmlElement* tag, const BinnerAxis& binner ) {
  const string & type = binner.name();
  tag->setAttribute ( m_type, type );

  const Range & range = binner.getRange ();
  double high = range.high();
  double low = range.low ();

  tag->setAttribute ( m_high, high );
  tag->setAttribute ( m_low, low );

  double width = binner.getBinWidth ();
  tag->setAttribute ( m_width, width );
}

typedef struct _XDisplay Display;
typedef unsigned long Window;
extern "C" void glFinish();
extern "C" void glXSwapBuffers(Display* display, Window window);

class OpenGLWindow {
  Display* m_display;
  Window m_window;
public:
  void flush();
};

// The wiki shows this example under wrapper without a label line
// @stereotype wrapper
void OpenGLWindow::flush()
{
  glFinish();
  glXSwapBuffers(m_display,m_window);
}
