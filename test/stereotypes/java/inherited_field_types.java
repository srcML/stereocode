// The type of an inherited data member counts against the class of the method that uses the data member

class Node {
    protected Element first;
    protected Node next;

    // @stereotype predicate
    boolean hasNext() { return next != null; }
}

class Element extends Node {
    // The data member has the type of the own class
    // @stereotype get
    Element first() { return first; }

    // The data member has the type of the parent, which is another class
    // @stereotype predicate collaborator
    boolean isLast() { return next == null; }
}
