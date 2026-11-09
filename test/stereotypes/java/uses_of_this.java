// The object itself: a method that returns this returns the object (Method-Stereotypes wiki, get, set, and command)

class Builder {
    int width;
    int height;

    // @stereotype get
    Builder self() { return this; }

    // A set or a command may return the object, as a method whose calls chain does
    // @stereotype set
    Builder width(int w) { this.width = w; return this; }

    // @stereotype command
    Builder size(int w, int h) { width = w; height = h; return this; }
}
