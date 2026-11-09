// The object itself: a method that returns this returns the object, and a write of this in a struct replaces the whole object
// (Method-Stereotypes wiki, get, set, and command)

struct Point
{
    int x;
    int y;

    // @stereotype command
    public void Reset(Point other) { this = other; }

    // @stereotype get
    public Point Self() { return this; }
}

class Builder
{
    int width;
    int height;

    // A set or a command may return the object, as a method whose calls chain does
    // @stereotype set
    public Builder Width(int w) { width = w; return this; }

    // @stereotype command
    public Builder Size(int w, int h) { width = w; height = h; return this; }
}
