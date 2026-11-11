// A call to a method of the own type counts by the stereotypes of that method: a call to an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object and counts like a call of a free function,
// and a call to any other method may change the object

// The incidental Bar is no accessor, and Test, which calls Bar, uses no state of the object
public class Foo
{
    // @stereotype incidental
    public int Bar() { return 4; }

    // @stereotype wrapper stateless
    public bool Test() { return Bar() + 5 > 0; }
}

public class Cell
{
    private int value;

    // @stereotype get
    public int Value() { return value; }

    // @stereotype empty
    public void Check() { }

    // @stereotype wrapper stateless
    public void Print() { Log.Write("cell"); }

    // @stereotype property wrapper
    public int Checked() { Check(); return value * 2; }

    // @stereotype wrapper stateless
    public void Refresh() { this.Check(); }

    // @stereotype property wrapper
    public int Printed() { Print(); return Value() + 1; }
}
