// Data members of known and unknown parents (Method-Stereotypes wiki, get, set, and command)

class Shape {
    protected int sides;
    protected int size;
}

class Square extends Shape {
    // @stereotype get
    int sides() { return sides; }
    // @stereotype set
    void setSides(int value) { sides = value; }
    // @stereotype command
    void grow() { sides++; size++; }
    // @stereotype command
    void reset() { super.clear(); }
}

// An unknown parent may declare the written data member
class Panel extends Widget {
    // @stereotype set
    void setLevel(int value) { level = value; }
}

// A name qualified by another class, as in Other.count, is no data member of the object, even when no type declares it
class Registry { static int count; }
class Recorder {
    int n;
    // @stereotype collaborator incidental
    void countOther() { Registry.count = 1; }
    // @stereotype collaborator incidental
    void countUnknown() { Journal.count = 1; }
}
