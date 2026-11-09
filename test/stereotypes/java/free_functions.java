// Free functions (Free-Function-Stereotypes wiki): static methods count as free functions
// predicate, property, global-command, command, factory, main, literal, wrapper, empty, and unclassified

final class Program {
    private static int calls;

    // @stereotype predicate
    public static boolean isEven(int value) { return value % 2 == 0; }

    // @stereotype property
    public static int square(int value) { return value * value; }

    // @stereotype global-command
    public static void addCalls(int count) { calls += count; }

    // @stereotype command
    public static void fill(int[] values, int value) { values[0] = value; }

    // @stereotype property factory
    public static Widget makeWidget(int style) { Widget widget = new Widget(style); return widget; }

    // @stereotype literal
    public static int answer() { return 42; }

    // @stereotype wrapper
    public static void report(int value) { Journal.write(value); }

    // @stereotype empty
    public static void noop(int value) { }

    // System.out.println is an ignored call, so a function that only reads its parameter fits no rule
    // @stereotype unclassified
    public static void print(int value) { System.out.println(value); }

    // @stereotype main
    public static void main(String[] args) { report(args.length); }
}
