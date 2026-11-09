// Constructors: locals are no data members, and a copy constructor takes the own class
// (Method-Stereotypes wiki, Creational)

using static Defaults;

// @stereotype boundary degenerate small-class
class Settings {
    private string path;
    // @stereotype constructor
    public Settings(string p) { string value = p.Trim(); path = value; }
    // The local value of the constructor is no data member, so the setter reads none
    public string Name {
        // @stereotype wrapper stateless
        set { Log.Write(value); } }
}

class Point {
    int x;
    // @stereotype constructor
    public Point(PointF p) { x = 1; }
    // @stereotype copy-constructor
    public Point(Point p) { x = p.x; }
}

// The local limit of the constructor is no data member, so Capacity reads the static limit of Defaults
class Buffer {
    private int size;
    // @stereotype constructor
    public Buffer() { int limit = 4; size = limit; }
    // @stereotype incidental
    public int Capacity() { return limit; }
}
