// A call through a chain of null-conditional accesses counts like the same chain written with . alone: after a data member or a parameter,
// a call on that variable, and after a call, a call on the result of the call
// (Method-Stereotypes wiki, command, controller, and collaborator, and Free-Function-Stereotypes wiki, command and wrapper)

class Holder {
    Node x;

    Node Current() { return x; }

    // @stereotype command collaborator
    void Chain() { x?.y?.Z(); }
    // @stereotype command collaborator
    void ChainPlain() { x.y.Z(); }
    // @stereotype command collaborator
    void ChainThenDot() { x?.y.Z(); }
    // @stereotype command collaborator
    void DotThenChain() { x.y?.Z(); }
    // @stereotype command collaborator
    void LongChain() { x?.y?.w?.Z(); }
    // @stereotype command collaborator
    void ChainThroughThis() { this.x?.y?.Z(); }

    // Next is called on the data member and Z on the result of Next
    // @stereotype command collaborator
    void CallInChain() { x?.Next()?.Z(); }
    // @stereotype command collaborator
    void CallInChainPlain() { x.Next().Z(); }

    // Current is an accessor of the type, and Z is called on its result
    // @stereotype controller
    void AfterCall() { Current()?.Z(); }
    // @stereotype controller
    void AfterCallChain() { Current()?.y?.Z(); }
    // @stereotype controller
    void AfterCallThenDot() { Current()?.y.Z(); }
}

// A parameter of a type that is not primitive shares the object of the caller, which a call through a chain changes, unless it calls a query method
static class Tools {
    // @stereotype command
    public static void Touch(Node p) { p?.y?.Z(); }
    // @stereotype command
    public static void TouchPlain(Node p) { p.y.Z(); }
    // @stereotype property wrapper
    public static int Size(Node p) { return p?.items?.Count() ?? 0; }
}
