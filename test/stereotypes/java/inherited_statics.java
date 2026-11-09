// A static method reaches the static fields of the ancestors of its type without their names, as a method does
// (Free-Function-Stereotypes wiki, global-command)

class Shelf {
    static int height;
}

class LowShelf extends Shelf {
    // @stereotype global-command
    static void lower() { height--; }
}

class Rack extends Shelf {
    static class Slot extends Holder {
        int depth;
        // A nested class reaches the static fields of the ancestors of the class around it, which no parent outside the archive declares
        // @stereotype incidental
        void raise() { height++; }
    }
}
