// A class of the archive with the name of a container of Java, as Stack, names its own type, unlike the container java.util.Stack

class Stack {
    int top;

    Stack(int start) { top = start; }

    // @stereotype get
    int peek() { return top; }

    // @stereotype property factory @defect the class counts as the container java.util.Stack
    static Stack create(int start) { Stack made = new Stack(start); return made; }
}

class Machine {
    Stack stack;
    int depth;

    // @stereotype factory collaborator stateless @defect the class counts as the container java.util.Stack
    Stack make() { return new Stack(); }

    // @stereotype property collaborator @defect the class counts as the container java.util.Stack
    Stack current() { return depth > 0 ? stack : null; }

    // @stereotype set collaborator @defect the class counts as the container java.util.Stack
    void take(Stack other) { depth = other.top; }
}
