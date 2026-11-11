// Large classes (Class-Stereotypes wiki, large-class): more methods than the mean plus the standard
// deviation of the methods per type, with accessors, mutators, controllers, and factories in balance
// The four classes have 0, 0, 23, and 22 methods, which give the threshold 11.25 + 11.26 = 22.51

public class Widget
{
}

public class Tag
{
}

// @stereotype large-class
public class Box
{
    private int width;
    private int height;
    private int depth;

    public int Width() { return width; }
    public int Height() { return height; }
    public int Depth() { return depth; }
    public int Area() { return width * height; }
    public int Volume() { return width * height * depth; }
    public int Perimeter() { return 2 * (width + height); }
    public void SetWidth(int value) { width = value; }
    public void SetHeight(int value) { height = value; }
    public void SetDepth(int value) { depth = value; }
    public void Resize(int value) { width = value; height = value; }
    public void Clear(int value) { width = value; height = value; }
    public void PaintFront(Widget widget) { widget.Poke(width + 0); }
    public void PaintBack(Widget widget) { widget.Poke(width + 1); }
    public void PaintLeft(Widget widget) { widget.Poke(width + 2); }
    public void PaintRight(Widget widget) { widget.Poke(width + 3); }
    public void PaintTop(Widget widget) { widget.Poke(width + 4); }
    public Widget MakeFront() { return new Widget(); }
    public Widget MakeBack() { return new Widget(); }
    public Widget MakeLeft() { return new Widget(); }
    public Widget MakeRight() { return new Widget(); }
    public Widget MakeTop() { return new Widget(); }
    public int Version() { return 2; }
    public int Revision() { return 7; }
}

// 22 methods in balance stay under the threshold of this archive
// @stereotype unclassified
public class Large22
{
    private int a;
    private int b;

    public int G1() { return a; }
    public int G2() { return a; }
    public int G3() { return a; }
    public int P4() { return a + 4; }
    public int P5() { return a + 5; }
    public int P6() { return a + 6; }
    public void S7(int v) { a = v; }
    public void S8(int v) { a = v; }
    public void S9(int v) { a = v; }
    public void C10(int v) { a = v; b = v; }
    public void C11(int v) { a = v; b = v; }
    public void K12(Widget w) { w.Poke(); }
    public void K13(Widget w) { w.Poke(); }
    public void K14(Widget w) { w.Poke(); }
    public void K15(Widget w) { w.Poke(); }
    public void K16(Widget w) { w.Poke(); }
    public Widget F17() { return new Widget(); }
    public Widget F18() { return new Widget(); }
    public Widget F19() { return new Widget(); }
    public Widget F20() { return new Widget(); }
    public Widget F21() { return new Widget(); }
    public int I22() { return 22; }
}
