// A cast before the returned object or data member is no computation, so the return stays a return of the object or of the data member
// (Method-Stereotypes wiki, get)

class Node {
    private Node next;
    private Object payload;
    private int timeout;

    // The type parameter T is no primitive type
    // @stereotype set collaborator
    <T> T timeout(int millis) { this.timeout = millis; return (T) this; }

    // @stereotype get collaborator
    <T> T self() { return (T) this; }

    // @stereotype get
    Node next() { return (Node) next; }

    // @stereotype get collaborator
    String text() { return (String) payload; }

    // A cast of a computed value stays a computation
    // @stereotype property
    long total() { return (long) timeout * 2; }
}
