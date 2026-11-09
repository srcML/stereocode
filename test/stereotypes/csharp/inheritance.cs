// Data members of known and unknown parents (Method-Stereotypes wiki, get, set, and command)

class Shape
{
    protected int sides;
    protected int size;
}

class Square : Shape
{
    // @stereotype get
    public int Sides() { return sides; }
    // @stereotype set
    public void SetSides(int value) { sides = value; }
    // @stereotype command
    public void Grow() { sides++; size++; }
    // @stereotype command
    public void Reset() { base.Clear(); }
}

// An unknown parent may declare the written data member
class Panel : Widget
{
    // @stereotype set
    public void SetLevel(int value) { level = value; }
}

// Static fields, const fields, and static properties are globals, also under an unknown parent
class Meter : Widget
{
    private static int instances;
    private const int Limit = 10;
    public static int Created { get; set; }
    // @stereotype incidental
    public void Count() { instances++; }
    // @stereotype incidental
    public void CountCreated() { Created++; }
    // @stereotype incidental
    public void CountQualified() { Meter.instances = 0; }
    // @stereotype incidental
    public int Max() { return Limit; }
    // @stereotype set
    public void SetLevel(int value) { level = value; }
}
