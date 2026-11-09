// Method-Stereotypes wiki, Degenerate, C++ examples (HippoDraw and Minuit), each in a minimal class

class NTupleChiSqFCN {
public:
  bool needsIntegrated() const;
};

// @stereotype incidental
bool NTupleChiSqFCN::needsIntegrated() const {
  return false;
}

// up() stands for the method of an unknown base class
double up();

class StatedFCN {
public:
// @stereotype wrapper stateless
virtual double errorDef () const {
    return up();
}
};

namespace hippodraw {
class Observable;
class Observer {
public:
  void willDelete(const Observable*);
};
}

// @stereotype empty
void hippodraw::Observer::willDelete(const Observable*) {
  // do nothing, default behaviour
}
