// Structures other than classes (run with -t for structs and -i for interfaces): the class rules name
// the structure, as in data-struct and small-interface

// @stereotype data-provider data-struct small-struct
public struct Point
{
    private int x;

    // @stereotype get
    public int GetX() { return x; }
}

// @stereotype empty
public interface IShape
{
    int Area();
}

// An interface has no data members, so a default method that returns the object itself is its get
// @stereotype data-provider data-interface small-interface
public interface IChain
{
    // @stereotype get
    IChain Self() { return this; }
}
