// Methods defined outside their class, under qualified names, and in template specializations:
// each gets the label of its in-class twin (Method-Stereotypes wiki)

#include <string>

class DataSource {
  int m_a;
  int m_b;
public:
  void copyPrivate(const DataSource& other);
  // @stereotype command
  void copyInside(const DataSource& other) { m_a = other.m_a; m_b = other.m_b; }
};

// The own class is no external object
// @stereotype command
void DataSource::copyPrivate(const DataSource& other) {
  m_a = other.m_a;
  m_b = other.m_b;
}

class Shape {
  int m_a;
public:
  Shape(const Shape& other);
  Shape* clonePtr() const;
};

// Labeled like an in-class clone, since passing *this to a constructor reads no state of the object
// @stereotype factory stateless
Shape* Shape::clonePtr() const { return new Shape(*this); }

class Widget {
    struct Impl;
    Impl* d;
public:
    int size() const;
};
struct Widget::Impl {
    int v;
    int get() const;
    // @stereotype get
    int getIn() const { return v; }
};
// @stereotype get
int Widget::Impl::get() const { return v; }
int Widget::size() const { return d->get(); }

namespace N { class C; }
class N::C {
    int w;
public:
    int getW() const;
};
// @stereotype get
int N::C::getW() const { return w; }

class Frame {
    class Impl;
    Impl* impl;
public:
    int size() const;
};
class Frame::Impl {
    int count;
public:
    // @stereotype get
    int get() const { return count; }
    void set(int c);
};
// @stereotype set
void Frame::Impl::set(int c) { count = c; }
int Frame::size() const { return impl->get(); }

template<typename T> class Box {
    T v;
public:
    void set(T x);
    T get() const { return v; }
};
// @stereotype set collaborator
template<> void Box<std::string>::set(std::string x) { v = x; }
// @stereotype set collaborator
template<> void Box<int>::set(int x) { v = x; }

template<typename T>
class Crate {
    T value;
    int count;
public:
    void reset();
    void bump();
    void touchIn() { count = 0; bump(); }
};
template<typename T>
void Crate<T>::reset() { count = 0; bump(); }
// @stereotype set
template<typename T>
void Crate<T>::bump() { count++; }
// @stereotype set
template<>
void Crate<std::string>::bump() { count += 2; }
template<>
void Crate<int>::reset() { count = 1; bump(); }
