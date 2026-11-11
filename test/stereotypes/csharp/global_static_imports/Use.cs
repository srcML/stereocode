// A name without a qualifier reaches the static fields of the types of the global static imports of any file
// (Free-Function-Stereotypes wiki, global-command, and Method-Stereotypes wiki, incidental)
namespace Shop
{
    class Limits
    {
        public static int Max;
    }
}

static class Use
{
    // @stereotype global-command
    public static void Cap() { Max = 3; }
}

class Basket : Collection
{
    // @stereotype incidental
    public void Raise() { Max = 5; }
}
