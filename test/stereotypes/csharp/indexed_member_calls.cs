// A call on an element of a data member is a call on the data member, and the declaration of the data member tells the type of its elements

using System.Collections.Generic;

class Slot {
    public int value;

    // @stereotype get
    public int Value() { return value; }

    // @stereotype set
    public void Clear() { value = 0; }
}

class Rack {
    private Slot[] slots;
    private Slot spare;
    private List<Slot> list;
    private Dictionary<string, Slot> named;

    // @stereotype command collaborator
    void ClearSpare() { spare.Clear(); }

    // @stereotype command collaborator
    void ClearAt(int i) { slots[i].Clear(); }

    // @stereotype command collaborator
    void ClearThrough(int i) { this.slots[i].Clear(); }

    // @stereotype property collaborator
    int ValueAt(int i) { return slots[i].Value() + 1; }

    // The elements of a generic collection have the type of its last type argument
    // @stereotype property collaborator
    int ListValue(int i) { return list[i].Value() + 1; }

    // @stereotype property collaborator
    int NamedValue(string name) { return named[name].Value() + 1; }

    // A call on an element of a parameter changes no data member
    // @stereotype controller stateless
    void ClearAll(Slot[] others) { others[0].Clear(); }

    // A call on an element of a parameter of a free function may change the object of the caller, as a call on the parameter itself may
    // @stereotype command
    static void ClearFirst(Slot[] others) { others[0].Clear(); }
}
