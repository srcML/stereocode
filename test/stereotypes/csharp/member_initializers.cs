// The member names of an object initializer, a with expression, and a property pattern, as Left of new Margin { Left = 1 }, name the
// members of another object, while the values after them are read

public class Inset { public int Top; }
public class Margin { public int Left; public Inset Inner; }

public class Page
{
    private int Left;
    private int Top;

    // @stereotype collaborator incidental
    public Margin Copy(Margin other) { return other with { Left = 1 }; }

    // @stereotype property collaborator
    public Margin Shift(Margin other) { return other with { Left = Left + 1 }; }

    // @stereotype collaborator incidental
    public bool IsFlush(Margin other) { return other is { Left: 0 }; }

    // @stereotype collaborator incidental
    public bool HasTop(Margin other) { return other is { Inner: { Top: 2 } }; }

    // @stereotype factory collaborator stateless
    public Margin Make() { return new Margin { Left = 1 }; }

    // @stereotype factory collaborator stateless
    public Margin Nested() { return new Margin { Inner = { Top = 2 } }; }

    // @stereotype property factory collaborator
    public Margin Uses() { return new Margin { Left = Left }; }

    // @stereotype factory collaborator stateless
    public object Named() { return new { Left = 1 }; }

    // A member of an anonymous type without a name takes the name and the value of the data member
    // @stereotype property factory collaborator
    public object Projected() { return new { Left }; }
}
