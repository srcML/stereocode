// The class of these methods is declared in a header that srcML parses as C, so stereocode warns and leaves the methods
// unclassified; with srcml --register-ext h=C++ they are set and get, as in the case header_parsed_as_cpp
#include "counter.h"

// @stereotype unclassified
void Counter::setCount(int c) {
  m_count = c;
}

// @stereotype unclassified
int Counter::count() const {
  return m_count;
}
