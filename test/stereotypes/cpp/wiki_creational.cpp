// Method-Stereotypes wiki, Creational, C++ example (HippoDraw) in a minimal class

class CutRangeRep {
  double m_low;
public:
  CutRangeRep(const CutRangeRep& other);
  CutRangeRep* clone();
};

// Passing *this to a constructor reads no state of the object
// @stereotype factory stateless
CutRangeRep* CutRangeRep::clone() {
  return new CutRangeRep(*this);
}
