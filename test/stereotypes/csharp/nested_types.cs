// Nested named types are types of their own under their qualified names, and their methods get stereotypes,
// while static types stay out (Class-Stereotypes wiki)

// @stereotype data-provider data-class small-class
public class Outer
{
    private int x;
    // @stereotype get
    public int GetX() { return x; }

    // @stereotype data-provider data-class small-class
    private class Inner
    {
        protected int v;
        // @stereotype get
        public int Get() { return v; }
    }

    // A parent named inside the outer type is a nested type of the outer type
    private class Counter : Inner
    {
        // @stereotype set
        public void Reset() { v = 0; }
    }

    // @stereotype data-provider data-struct small-struct
    private struct Pair
    {
        private int first;
        // @stereotype get
        public int First() { return first; }
    }
}

public class Inner
{
    private int other;
}

public class Counter
{
    private int count;
    private static int created;

    // A nested class reaches the data members of the outer type only through an object of the outer type,
    // so they are no data members of the nested class
    private class Step
    {
        private Counter owner;
        // @stereotype set collaborator
        public void Bump() { owner.count++; }
        // @stereotype property collaborator
        public int Peek() { return owner.count + 1; }
        // A static data member of the outer type is a global
        // @stereotype incidental
        public void Note() { created++; }
    }
}

public static class Registry
{
    // A class nested in a static class is a type of its own
    // @stereotype data-provider data-class small-class
    public class Entry
    {
        private string key;
        // @stereotype get
        public string Key() { return key; }
    }
}
