// A call to the object itself that names no method of the archive names a method of a parent outside the archive, since C# has no
// free functions: a query method reads the object, and any other method may change it (Method-Stereotypes wiki, Calls)
using System;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;

class Names : Collection<string>
{
    private int resets;

    // @stereotype command
    void Reset() { Clear(); resets++; }

    // @stereotype command
    void Drop() { base.Clear(); }

    // @stereotype predicate
    bool Has(string name) { return Contains(name); }

    // @stereotype predicate
    bool HasThis(string name) { return this.Contains(name); }

    // @stereotype command
    void Put(string name) { Add(name); }

    // A static method of the type is a free function
    // @stereotype wrapper stateless
    int Four() { return Twice(2); }

    static int Twice(int value) { return 2 * value; }

    private bool IsLong(string name) => name.Length > 8;

    private int Limit() => 8;

    [DllImport("kernel32")] private static extern bool Beep(int frequency, int duration);

    // srcML 1.2 marks an expression-bodied method as another element, which gets no stereotype, so its call stays a call of a function
    // @stereotype wrapper stateless
    bool Fits(string name) { return IsLong(name) && name.Length < Limit(); }

    // A static method without a body is a function as well
    // @stereotype wrapper stateless
    void Signal() { Beep(440, 100); }

    // A delegate that the method calls by its name is no method of a parent
    // @stereotype wrapper stateless
    void Run(Action step) { step(); }

    class Cursor
    {
        // A static method of a type around the type is a free function as well
        // @stereotype wrapper stateless
        int Eight() { return Twice(4); }
    }
}

// Every class inherits the accessors of object, which read the object
class Point
{
    private int x;
    private string name;

    // @stereotype property
    public override string ToString() { return base.ToString() + "[" + name + "]"; }

    // @stereotype property
    public override int GetHashCode() { return 31 * base.GetHashCode() + x; }

    // @stereotype predicate collaborator
    public override bool Equals(object other) { return base.Equals(other) && x == ((Point) other).x; }

    // @stereotype property
    public Point Copy() { return (Point) MemberwiseClone(); }

    // @stereotype property
    string Describe() { return this.ToString(); }

    // @stereotype predicate controller
    bool SameType(object other) { return other != null && GetType() == other.GetType(); }

    // The static method ReferenceEquals of object is a free function
    // @stereotype predicate wrapper
    bool Identical(object other) { return ReferenceEquals(this, other); }
}

// A dependency object keeps the values of its properties, which GetValue reads and SetValue changes
class Popup : DependencyObject {
    public static readonly DependencyProperty IsOpenProperty = DependencyProperty.Register("IsOpen", typeof(bool), typeof(Popup));

    public bool IsOpen {
        // @stereotype predicate
        get { return (bool)GetValue(IsOpenProperty); }

        // @stereotype command
        set { SetValue(IsOpenProperty, value); }
    }
}

class Trackers {
    private static readonly ConditionalWeakTable<object, Popup> s_popups = new ConditionalWeakTable<object, Popup>();

    // GetValue of a ConditionalWeakTable adds a missing key through its callback
    // @stereotype global-command wrapper
    static void Track(object owner) { s_popups.GetValue(owner, key => new Popup()); }
}
