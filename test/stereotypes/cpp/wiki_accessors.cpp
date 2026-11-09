// Method-Stereotypes wiki, Structural Accessors, C++ examples (HippoDraw and FLTK), each in a minimal class

class AxisModelBase {
  double m_tick_step;
public:
  double getTickStep() const;
};

// @stereotype get
double AxisModelBase::getTickStep() const {
  return m_tick_step;
}

class RootBranch {
  bool m_useable;
  unsigned int m_number_elements;
public:
  bool isUseable() const;
  bool isMultiDimensional() const;
};

// @stereotype get
bool RootBranch::isUseable() const {
  return m_useable;
}

// @stereotype predicate
bool RootBranch::isMultiDimensional() const {
  return !(m_number_elements == 1 );
}

class RepBase {
  bool m_desel;
public:
  bool isSelected() const;
};

// @stereotype predicate
bool RepBase::isSelected() const {
  return m_desel == false;
}

class Range {
  double m_min;
  double m_max;
public:
  double fraction(double value) const;
  double length() const;
};

// @stereotype property
double Range::fraction(double value) const {
  return (value - m_min)/(m_max - m_min);
}

// @stereotype property
inline double Range::length() const {
  return (m_max - m_min);
}

class Fl_Table {
  int toprow;
  int botrow;
  int leftcol;
  int rightcol;
public:
// @stereotype void-accessor
inline void visible_cells(int& r1, int& r2, int& c1, int& c2) {
    r1 = toprow;
    r2 = botrow;
    c1 = leftcol;
    c2 = rightcol;
}
};
