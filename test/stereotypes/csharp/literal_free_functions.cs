// A literal uses no parameter, no global, and no static (Free-Function-Stereotypes wiki, literal, global-command, and wrapper)

public static class Messages
{
    static string s_name = "stereocode";

    // @stereotype literal wrapper
    public static void Greet() { Journal.Write(42); }

    // @stereotype wrapper
    public static void Show() { Journal.Write(s_name); }

    // @stereotype global-command
    public static void Clear() { s_name = ""; }
}
