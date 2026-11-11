// Large classes (Class-Stereotypes wiki, large-class): more methods than the mean plus the standard
// deviation of the methods per type, with accessors, mutators, controllers, and factories in balance
// The four classes have 0, 0, 23, and 22 methods, which give the threshold 11.25 + 11.26 = 22.51

class Widget {
}

class Tag {
}

// @stereotype large-class
class Box {
    private int width;
    private int height;
    private int depth;

    int width() { return width; }
    int height() { return height; }
    int depth() { return depth; }
    int area() { return width * height; }
    int volume() { return width * height * depth; }
    int perimeter() { return 2 * (width + height); }
    void setWidth(int value) { width = value; }
    void setHeight(int value) { height = value; }
    void setDepth(int value) { depth = value; }
    void resize(int value) { width = value; height = value; }
    void clear(int value) { width = value; height = value; }
    void paintFront(Widget widget) { widget.poke(width + 0); }
    void paintBack(Widget widget) { widget.poke(width + 1); }
    void paintLeft(Widget widget) { widget.poke(width + 2); }
    void paintRight(Widget widget) { widget.poke(width + 3); }
    void paintTop(Widget widget) { widget.poke(width + 4); }
    Widget makeFront() { return new Widget(); }
    Widget makeBack() { return new Widget(); }
    Widget makeLeft() { return new Widget(); }
    Widget makeRight() { return new Widget(); }
    Widget makeTop() { return new Widget(); }
    int version() { return 2; }
    int revision() { return 7; }
}

// 22 methods in balance stay under the threshold of this archive
// @stereotype unclassified
class Large22 {
    private int a;
    private int b;

    int g1() { return a; }
    int g2() { return a; }
    int g3() { return a; }
    int p4() { return a + 4; }
    int p5() { return a + 5; }
    int p6() { return a + 6; }
    void s7(int v) { a = v; }
    void s8(int v) { a = v; }
    void s9(int v) { a = v; }
    void c10(int v) { a = v; b = v; }
    void c11(int v) { a = v; b = v; }
    void k12(Widget w) { w.poke(); }
    void k13(Widget w) { w.poke(); }
    void k14(Widget w) { w.poke(); }
    void k15(Widget w) { w.poke(); }
    void k16(Widget w) { w.poke(); }
    Widget f17() { return new Widget(); }
    Widget f18() { return new Widget(); }
    Widget f19() { return new Widget(); }
    Widget f20() { return new Widget(); }
    Widget f21() { return new Widget(); }
    int i22() { return 22; }
}
