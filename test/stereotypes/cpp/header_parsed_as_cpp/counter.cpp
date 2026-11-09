#include "counter.hpp"

// @stereotype set
void Counter::setCount(int c) {
  m_count = c;
}

// @stereotype get
int Counter::count() const {
  return m_count;
}
