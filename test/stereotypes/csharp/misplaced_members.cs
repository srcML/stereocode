// srcML 1.2 nests the members that follow a switch expression in the function that holds the expression
// A nested function with an access modifier is no local function, so its code is no code of the function around it

class Formats
{
    private static int picks;

    // @stereotype property
    private static int Pick(string name)
    {
        int width = name switch
        {
            "wide" => 2,
            _ => 1
        };
        return width;
    }

    // @stereotype global-command
    internal static void Count()
    {
        picks++;
    }

    // @stereotype property
    internal static int Twice(int value)
    {
        int Double(int number) { return 2 * number; }
        return Double(value);
    }
}
