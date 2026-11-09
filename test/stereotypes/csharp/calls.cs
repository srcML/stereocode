// Calls on call results and casts, null-conditional calls, calls through this and base, and ignored calls
// (Method-Stereotypes wiki, controller, command, and incidental)

class Pipe {
    // @stereotype controller stateless
    void Close(object o) { ((System.IDisposable)o).Dispose(); }
    // @stereotype controller stateless
    void Run(Helper h) { h.Next().Run(); }
}

class Base { public virtual void Draw() { } }
class Canvas : Base {
    int x;
    // @stereotype set
    public void G() { x = 1; }
    // @stereotype command
    public void CallPlain() { G(); }
    // @stereotype command
    public void CallThis() { this.G(); }
    // The empty Draw of Base is a degenerate method, whose call counts like a call of a free function
    // @stereotype wrapper stateless
    public override void Draw() { base.Draw(); }
    // Console.WriteLine is an ignored call, also fully qualified
    // @stereotype incidental
    public void Log() { System.Console.WriteLine("x"); }
    // @stereotype incidental
    public void Log2() { Console.WriteLine("x"); }
    // Assertions and messages to the debugger or the error stream are ignored calls
    // @stereotype incidental
    public void Check(int v) { Debug.Assert(v > 0); Debug.WriteLine("v"); Console.Error.WriteLine("v"); }
}

class Customer {
    string _name;
    // nameof is no call
    // @stereotype get
    string GetName(int i) { if (i < 0) throw new ArgumentOutOfRangeException(nameof(i)); return _name; }
}

class Resource { public void Dispose() { } public Resource Next() { return this; } }
class Holder {
    Resource o;
    // A null-conditional call on a field is a call on the field, as o.Dispose() is, and the empty Dispose of Resource is a degenerate method,
    // whose call counts like a call of a free function
    // @stereotype wrapper
    void Release() { o?.Dispose(); }
    // @stereotype wrapper
    void ReleasePlain() { o.Dispose(); }
    // Next of Resource returns the object itself, a get that reads the field, and Dispose is called on its result, another object
    // @stereotype controller
    void ReleaseNext() { o?.Next().Dispose(); }
    // @stereotype controller
    void ReleaseNextPlain() { o.Next().Dispose(); }
}

// A call of a query method on a data member reads the data member
class Inventory {
    List<int> items;
    Dictionary<string, int> prices;
    // @stereotype property
    int Total() { return items.Count(); }
    // @stereotype predicate
    bool Has(string name) { return prices.ContainsKey(name); }
    // @stereotype property
    string Describe() { return items.ToString(); }
    // @stereotype command
    void Add(int x) { items.Add(x); }
}
