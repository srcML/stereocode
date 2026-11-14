// A call on a ref or out parameter or on a parameter of a type that is not primitive changes the object of the caller,
// unless it calls a query method, and a call on a static field changes a global
// (Free-Function-Stereotypes wiki, command, global-command, and wrapper)
using System.Collections.Generic;

public struct Point
{
    public int X;
    public void Move() { X++; }
}

public struct Slot<T>
{
    public T Item;
    public int Hits;
    public void Touch() { Hits++; }
}

public enum Color { Red, Green }

public static class Lists
{
    static List<int> s_log = new List<int>();
    const string Prefix = "p";
    static readonly string Name = "n";
    static readonly System.Text.StringBuilder Shared = new System.Text.StringBuilder();

    // @stereotype command
    public static void Fill(List<int> items, int value) { items.Add(value); }

    // @stereotype command
    public static void Bump(ref Counter counter) { counter.Increment(); }

    // A struct or an enum of the archive passed by value or as an in parameter holds no object of the caller that a call may change
    // @stereotype wrapper
    public static void MoveCopy(Point point) { point.Move(); }

    // @stereotype wrapper
    public static void MoveIn(in Point point) { point.Move(); }

    // @stereotype command
    public static void MoveRef(ref Point point) { point.Move(); }

    // An array holds the elements of the caller
    // @stereotype command
    public static void Reset(int[] items) { items.SetValue(0, 0); }

    // An in parameter of a class holds the object of the caller, whose reference alone is read-only
    // @stereotype command
    public static void FillIn(in List<int> items, int value) { items.Add(value); }

    // @stereotype wrapper
    public static void TouchCopy(Slot<string> slot) { slot.Touch(); }

    // @stereotype command
    public static void TouchRef(ref Slot<string> slot) { slot.Touch(); }

    // @stereotype predicate wrapper
    public static bool IsRed(Color color) { return color.HasFlag(Color.Red); }

    // A query method reads the parameter
    // @stereotype predicate wrapper
    public static bool Has(List<int> items, int value) { return items.Contains(value); }

    // A parameter of a primitive type holds no object of the caller
    // @stereotype property wrapper
    public static int Compare(int n) { return n.CompareTo(0); }

    // @stereotype global-command wrapper
    public static void Log(int value) { s_log.Add(value); }

    // A call on a constant or on a readonly static field of a primitive type or string changes no global
    // @stereotype wrapper
    public static string Upper() { return Prefix.ToUpper(); }

    // @stereotype wrapper
    public static string Lower() { return Name.ToLower(); }

    // @stereotype global-command wrapper
    public static void Append(string text) { Shared.Append(text); }
}
