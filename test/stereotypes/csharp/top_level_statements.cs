// A function among the top-level statements of a program is a free function, while a function after a type shows damaged markup
System.Console.WriteLine(Twice(2) + Thrice(3));

// @stereotype property
int Twice(int x) { return x * 2; }

// @stereotype property
static int Thrice(int x) { return x * 3; }

public class Holder
{
    private int count;

    // @stereotype get
    public int Count() { return count; }
}
