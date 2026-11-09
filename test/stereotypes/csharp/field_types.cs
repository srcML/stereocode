// Data members of the own generic type and of collection interfaces are no objects of other classes
// (Method-Stereotypes wiki, get and collaborator)

using System.Collections.Generic;

public class Node<T> {
    private Node<T> next;
    // @stereotype get
    public Node<T> GetNext() { return next; }
}

public class Bag {
    private List<int> list;
    private IEnumerable<int> seq;
    // @stereotype get
    public List<int> GetList() { return list; }
    // @stereotype get
    public IEnumerable<int> GetSeq() { return seq; }
}
