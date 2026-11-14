// The type of an inherited data member counts against the class of the method that uses the data member

public class Node
{
    protected Element first;
    protected Node next;

    // @stereotype predicate
    public bool HasNext() { return next != null; }
}

public class Element : Node
{
    // The data member has the type of the own class
    // @stereotype get
    public Element First() { return first; }

    // The data member has the type of the parent, which is another class
    // @stereotype predicate collaborator
    public bool IsLast() { return next == null; }
}
