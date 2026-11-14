// C-style array declarators name the same variable as Java-style ones
// (Method-Stereotypes wiki, incidental; Free-Function-Stereotypes wiki, command)

class Arr {
    int x;
    // @stereotype incidental
    void localCStyle() { int a[] = new int[3]; a[0] = 1; }
    // @stereotype incidental
    void localJavaStyle() { int[] a = new int[3]; a[0] = 1; }
    // @stereotype command
    static void paramCStyle(int a[]) { a[0] = 1; }
    // @stereotype command
    static void paramJavaStyle(int[] a) { a[0] = 1; }

    // A variadic parameter takes its arguments as an array, which the caller may pass
    // @stereotype command
    static void paramVariadic(int... a) { a[0] = 1; }
    // @stereotype void-accessor
    void copyToArray(int[] out) { out[0] = x; }
    // @stereotype void-accessor
    void copyToVariadic(int... out) { out[0] = x; }
}
