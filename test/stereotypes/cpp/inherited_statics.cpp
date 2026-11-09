// A static method reaches the static data members of the ancestors of its type without their names, as a method does
// (Free-Function-Stereotypes wiki, global-command)

class Shelf {
protected:
    static int height;
};

class LowShelf : public Shelf {
public:
    static void lower();
};

// @stereotype global-command
void LowShelf::lower() { height--; }
