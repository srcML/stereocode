// Degenerate methods (Method-Stereotypes wiki): incidental, stateless, and empty

public class Session
{
    private int id;

    // @stereotype incidental
    public int Version() { return 3; }

    // @stereotype wrapper stateless
    public int Limit(int value) { return Bounds.Clamp(value); }

    // @stereotype controller stateless
    public void Notify(Listener listener) { listener.Changed(); }

    // @stereotype empty
    public void Refresh() { }

    // @stereotype empty
    public void Close() { /* nothing to release */ }
}
