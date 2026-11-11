// A copy constructor takes the own class as its one parameter (Method-Stereotypes wiki, Creational)

class Node {
    int value;

    // @stereotype constructor
    public Node(Node parent, int value) {
        this.value = value;
    }

    // @stereotype get
    public int getValue() {
        return value;
    }
}

class Order {
    private int id;
    // @stereotype constructor
    Order(OrderBook book) { id = book.next(); }
}

class Vertex {
    int kind;
    // @stereotype constructor
    Vertex(VertexKind k) { kind = 1; }
    // @stereotype copy-constructor
    Vertex(Vertex other) { kind = other.kind; }
}
