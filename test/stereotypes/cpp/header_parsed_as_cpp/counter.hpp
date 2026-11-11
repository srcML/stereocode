// A .hpp file is C++ for srcML, as a .h file is with --register-ext h=C++
// @stereotype data-class small-class
class Counter {
  int m_count;
public:
  void setCount(int c);
  int count() const;
};
