// A write through the result of a call, as in Store.Shared().slots[i].b = v, writes the object that the call gives,
// so the names after the call name members of that object, never a data member or a parameter of the same name
// (Method-Stereotypes wiki, set, command, wrapper, and controller, and Free-Function-Stereotypes wiki, command and wrapper)

class Slot {
    public int b;
}

class Part {
    public int b;
    public string name;
    public Slot[] slots;
}

class Store {
    public static Part Shared() { return null; }
    public static Part[] Parts() { return null; }
    public static int Index() { return 0; }
}

class Holder {
    private int b;
    private string name;
    private int[] slots;

    // @stereotype get
    public Holder Self() { return this; }

    // The member b of the part that Store.Shared() gives takes the value of the data member b, which stays as it is
    // @stereotype wrapper
    public void CopyToPart() { Store.Shared().b = b; }

    // @stereotype wrapper
    public void CopyToSlot(int i) { Store.Shared().slots[i].b = b; }

    // @stereotype wrapper
    public void CopyToElement(int i) { Store.Parts()[i].b = b; }

    // @stereotype wrapper
    public void AddToPart() { Store.Shared().b += b; }

    // @stereotype wrapper
    public void NamePart() { Store.Shared().name ??= name; }

    // A null-conditional write after a call, as Store.Shared()?.b = v, writes through the result of the call as well
    // @stereotype wrapper
    public void CopyToPartIfAny() { Store.Shared()?.b = b; }

    // @stereotype wrapper
    public void CopyToSlotIfAny() { Store.Shared()?.slots?[0].b = b; }

    // A null-conditional write after a parameter writes a member of the parameter, as the same write with . does
    // @stereotype void-accessor controller
    public void CopyToGiven(Part given) { given.b = b; }

    // @stereotype void-accessor controller
    public void CopyToGivenIfAny(Part given) { given?.b = b; }

    // A write through the result of a call on another object changes that object
    // @stereotype controller
    public void CopyToOther(Holder other) { other.Self().b = b; }

    // A write through the result of an accessor of the own type changes the object, also with ??= and after ?.
    // @stereotype command
    public void NameItself() { Self().name ??= "holder"; }

    // @stereotype command
    public void NameItselfIfAny() { Self()?.name = "holder"; }

    // A chain that starts with a data member writes the data member, also with a call in its index
    // @stereotype set
    public void SetSlot() { slots[Store.Index()] = b; }

    // A static method that writes through the result of a call changes no parameter of the same name
    // @stereotype wrapper
    public static void CopyBack(ref int b) { Store.Shared().b = b; }
}
