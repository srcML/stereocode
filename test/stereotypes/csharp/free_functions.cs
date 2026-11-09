// Free functions (Free-Function-Stereotypes wiki): static methods count as free functions
// predicate, property, global-command, command, factory, main, literal, wrapper, and empty

public static class Program
{
    private static int calls;

    // @stereotype predicate
    public static bool IsEven(int value) { return value % 2 == 0; }

    // @stereotype property
    public static int Square(int value) { return value * value; }

    // @stereotype global-command
    public static void AddCalls(int count) { calls += count; }

    // @stereotype command
    public static void Swap(ref int a, ref int b) { int t = a; a = b; b = t; }

    // @stereotype property factory
    public static Widget MakeWidget(int style) { Widget widget = new Widget(style); return widget; }

    // @stereotype literal
    public static int Answer() { return 42; }

    // @stereotype wrapper
    public static void Report(int value) { Journal.Write(value); }

    // @stereotype empty
    public static void Noop(int value) { }

    // @stereotype main
    public static void Main(string[] args) { Report(args.Length); }
}
