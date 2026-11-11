// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

// @stereotype data-class small-class
class Outer {
    private int a;
    // @stereotype data-provider data-class small-class
    static class Nested { int b;
    // @stereotype get
    int getB() { return b; } }
    // @stereotype get
    int getA() { return a; }
    // @stereotype set
    void setA(int value) { a = value; }
}
