// Large classes (Class-Stereotypes wiki, large-class): more methods than the mean plus the standard
// deviation of the methods per type, with accessors, mutators, controllers, and factories in balance
// The four classes have 0, 23, 22, and 0 methods, which give the threshold 11.25 + 11.26 = 22.51

class Widget {
public:
    void poke();
    void poke(int value);
};

class Tag {
};

// @stereotype large-class
class Box {
    int m_width;
    int m_height;
    int m_depth;
public:
    int width() const { return m_width; }
    int height() const { return m_height; }
    int depth() const { return m_depth; }
    int area() const { return m_width * m_height; }
    int volume() const { return m_width * m_height * m_depth; }
    int perimeter() const { return 2 * (m_width + m_height); }
    void setWidth(int value) { m_width = value; }
    void setHeight(int value) { m_height = value; }
    void setDepth(int value) { m_depth = value; }
    void resize(int value) { m_width = value; m_height = value; }
    void clear(int value) { m_width = value; m_height = value; }
    void paintFront(Widget* widget) { widget->poke(m_width + 0); }
    void paintBack(Widget* widget) { widget->poke(m_width + 1); }
    void paintLeft(Widget* widget) { widget->poke(m_width + 2); }
    void paintRight(Widget* widget) { widget->poke(m_width + 3); }
    void paintTop(Widget* widget) { widget->poke(m_width + 4); }
    Widget* makeFront() { return new Widget(); }
    Widget* makeBack() { return new Widget(); }
    Widget* makeLeft() { return new Widget(); }
    Widget* makeRight() { return new Widget(); }
    Widget* makeTop() { return new Widget(); }
    int version() const { return 2; }
    int revision() const { return 7; }
};

// 22 methods in balance stay under the threshold of this archive
// @stereotype unclassified
class Large22 {
  int m_a;
  int m_b;
public:
  int g1() const { return m_a; }
  int g2() const { return m_a; }
  int g3() const { return m_a; }
  int p4() const { return m_a + 4; }
  int p5() const { return m_a + 5; }
  int p6() const { return m_a + 6; }
  void s7(int v) { m_a = v; }
  void s8(int v) { m_a = v; }
  void s9(int v) { m_a = v; }
  void c10(int v) { m_a = v; m_b = v; }
  void c11(int v) { m_a = v; m_b = v; }
  void k12(Widget* w) { w->poke(); }
  void k13(Widget* w) { w->poke(); }
  void k14(Widget* w) { w->poke(); }
  void k15(Widget* w) { w->poke(); }
  void k16(Widget* w) { w->poke(); }
  Widget* f17() { return new Widget(); }
  Widget* f18() { return new Widget(); }
  Widget* f19() { return new Widget(); }
  Widget* f20() { return new Widget(); }
  Widget* f21() { return new Widget(); }
  int i22() const { return 22; }
};
