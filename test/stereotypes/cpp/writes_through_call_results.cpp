// A write through the result of a call, as in shared().slots[i].b = v, writes the object that the call gives,
// so the names after the call name members of that object, never a data member, a parameter, or a global of the same name
// (Method-Stereotypes wiki, set, command, wrapper, and controller, and Free-Function-Stereotypes wiki, command and wrapper)

struct Slot {
    int b;
};

struct Part {
    int  b;
    int  g;
    Slot slots[4];
};

Part& shared();
Part* parts();
int   index();
int   g;

class Holder {
    int b;
    int slots[4];
public:
    // @stereotype get
    Holder& self() { return *this; }

    // The member b of the part that shared() gives takes the value of the data member b, which stays as it is
    // @stereotype wrapper
    void copyToPart() { shared().b = b; }

    // @stereotype wrapper
    void copyToSlot(int i) { shared().slots[i].b = b; }

    // @stereotype wrapper
    void copyToFirstSlot() { shared().slots->b = b; }

    // @stereotype wrapper
    void copyToElement(int i) { parts()[i].slots[i].b = b; }

    // @stereotype wrapper
    void copyThroughPointer() { parts()->b = b; }

    // @stereotype wrapper
    void addToPart() { shared().b += b; }

    // @stereotype wrapper
    void bumpSlot() { shared().slots[b].b++; }

    // A write through the result of a call on another object changes that object
    // @stereotype controller
    void copyToOther(Holder& other) { other.self().b = b; }

    // A write through the result of an accessor of the own type changes the object
    // @stereotype command
    void clearOwn() { self().slots[0] = 0; }

    // A chain that starts with a data member writes the data member, also with a call in its index
    // @stereotype set
    void setSlot() { slots[index()] = b; }
};

// A free function that writes through the result of a call changes no parameter and no global of the same name
// @stereotype wrapper
void copyBack(int& b) { shared().b = b; }

// @stereotype wrapper
void copyGlobal() { shared().g = g; }

// A chain that starts with a parameter writes the object of the parameter
// @stereotype command
void clearFirst(Part& part) { part.slots[index()].b = 0; }
