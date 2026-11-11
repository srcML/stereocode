// A literal uses no parameter, no global, and no static (Free-Function-Stereotypes wiki, literal, global-command, and wrapper)

final class Messages {
    static final String NAME = "stereocode";
    static int shown;

    // @stereotype literal wrapper
    static void greet() { Journal.write(42); }

    // @stereotype wrapper
    static void show() { Journal.write(NAME); }

    // @stereotype global-command
    static void reset() { shown = 0; }
}
