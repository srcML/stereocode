// C# out and ref are parameter modifiers, not parts of a type name (Free-Function-Stereotypes wiki, command;
// Method-Stereotypes wiki, void-accessor)

class Layout { }
class Holder {
    int x;
    // A by-value parameter whose type name contains "out" is no out parameter
    // @stereotype unclassified
    static void ResetLayout(Layout l) { l = null; }
    // @stereotype unclassified
    static void ResetPlain(Holder h) { h = null; }
    // @stereotype void-accessor
    void PeekOut(out int r) { r = x; }
}
