// Large and lazy structures (run with -t for structs and -i for interfaces): the class rules name the structure,
// as in large-struct and lazy-interface (Class-Stereotypes wiki, large-class and lazy-class)
// The only class, Widget, has no methods, which gives the large-class threshold 0

public class Widget
{
}

// Four methods, more than the threshold, with a get, a set, a controller, and a factory in balance
// @stereotype large-struct
public struct Frame
{
    private int width;

    // @stereotype get
    public int GetWidth() { return width; }

    // @stereotype set
    public void SetWidth(int value) { width = value; }

    // @stereotype controller
    public void Paint(Widget widget) { widget.Poke(width); }

    // @stereotype factory collaborator stateless
    public Widget Make() { return new Widget(); }
}

// Two of the four methods are degenerate, more than one third, and the others are a get and a set
// @stereotype lazy-struct
public struct Slot
{
    private int value;

    // @stereotype get
    public int Get() { return value; }

    // @stereotype set
    public void Set(int v) { value = v; }

    // @stereotype empty
    public void Reset() { }

    // @stereotype empty
    public void Clear() { }
}

// An interface has no data members, so its default methods return the object itself or call its abstract methods
// @stereotype large-interface
public interface IShape
{
    void Clear();

    // @stereotype get
    IShape Self() { return this; }

    // The abstract Clear has no stereotypes, so its call may change the object
    // @stereotype command
    void Reset() { Clear(); }

    // The call of the accessor Self reads the object
    // @stereotype controller
    void Paint(Widget widget) { widget.Poke(Self()); }

    // @stereotype factory collaborator stateless
    Widget Make() { return new Widget(); }
}

// @stereotype data-provider lazy-interface
public interface ILazy
{
    // @stereotype get
    ILazy Self() { return this; }

    // @stereotype get
    ILazy Current() { return this; }

    // @stereotype empty
    void Reset() { }

    // @stereotype empty
    void Clear() { }
}
