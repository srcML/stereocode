// A static method reaches the static fields of the ancestors of its type without their names, as a method does
// (Free-Function-Stereotypes wiki, global-command)

class Shelf
{
    protected static int Height;
}

class LowShelf : Shelf
{
    // @stereotype global-command
    public static void Lower() { Height--; }
}
