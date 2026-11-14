// A call on an element of a data member is a call on the data member, and the declaration of the data member tells the type of its elements

class Slot {
    int value;

    // @stereotype get
    int value() { return value; }

    // @stereotype set
    void clear() { value = 0; }
}

class Rack {
    private Slot[] slots;
    private Slot[][] grid;
    private Slot spare;
    private java.util.List<Slot> list;

    // @stereotype command collaborator
    void clearSpare() { spare.clear(); }

    // @stereotype command collaborator
    void clearAt(int i) { slots[i].clear(); }

    // @stereotype command collaborator
    void clearThrough(int i) { this.slots[i].clear(); }

    // @stereotype property collaborator
    int valueAt(int i) { return slots[i].value() + 1; }

    // @stereotype property collaborator
    int valueIn(int i, int j) { return grid[i][j].value() + 1; }

    // A call on the result of a call is a call on another object
    // @stereotype property controller
    int listValue(int i) { return list.get(i).value(); }

    // A call on an element of a parameter changes no data member
    // @stereotype controller stateless
    void clearAll(Slot[] others) { others[0].clear(); }

    // A call on an element of a parameter of a free function may change the object of the caller, as a call on the parameter itself may
    // @stereotype command
    static void clearFirst(Slot[] others) { others[0].clear(); }
}
