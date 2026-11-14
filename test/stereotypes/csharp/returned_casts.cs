// A cast before the returned object or data member is no computation, so the return stays a return of the object or of the data member
// (Method-Stereotypes wiki, get)

public class Node {
    private Node next;
    private object payload;
    private int timeout;

    // The type parameter T is no primitive type
    // @stereotype set collaborator
    public T Timeout<T>(int millis) where T : Node { this.timeout = millis; return (T)this; }

    // @stereotype get
    public Node Next() { return (Node)next; }

    // @stereotype get collaborator
    public string Text() { return (string)payload; }

    // @stereotype property
    public long Total() { return (long)timeout * 2; }
}
