// A write through the result of a call, as in Store.shared().slots[i].b = v, writes the object that the call gives,
// so the names after the call name members of that object, never a data member or a parameter of the same name
// (Method-Stereotypes wiki, set, command, wrapper, and controller, and Free-Function-Stereotypes wiki, command and wrapper)

class Slot {
    int b;
}

class Part {
    int b;
    Slot[] slots;

    Part(int b) { this.b = b; }
}

class Store {
    static Part shared() { return null; }
    static Part[] parts() { return null; }
    static int scale(int b) { return 2 * b; }
}

class Holder {
    private int b;
    private int[] slots;

    // @stereotype get
    Holder self() { return this; }

    // The member b of the part that Store.shared() gives takes the value of the data member b, which stays as it is
    // @stereotype wrapper
    void copyToPart() { Store.shared().b = b; }

    // @stereotype wrapper
    void copyToSlot(int i) { Store.shared().slots[i].b = b; }

    // @stereotype wrapper
    void copyToElement(int i) { Store.parts()[i].b = b; }

    // @stereotype wrapper
    void bumpSlot() { Store.shared().slots[b].b++; }

    // A write to a member of the object that new creates leaves the data member of the same name as it is
    // @stereotype wrapper
    void fillNewPart() { new Part(0).b = Store.scale(b); }

    // A write through the result of a call on another object changes that object
    // @stereotype controller
    void copyToOther(Holder other) { other.self().b = b; }

    // A write through the result of an accessor of the own type changes the object
    // @stereotype command
    void clearOwn() { this.self().slots[0] = 0; }

    // A chain that starts with a data member writes the data member, also with a call in its index
    // @stereotype set
    void setSlot() { slots[Store.scale(b)] = b; }

    // A static method that writes through the result of a call changes no parameter of the same name
    // @stereotype wrapper
    static void copyBack(int[] slots) { Store.shared().slots[0].b = slots[0]; }
}
