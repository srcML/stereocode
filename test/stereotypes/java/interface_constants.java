// Interface constants are static, as static class constants are, and static data members are no
// object state for the tool (Method-Stereotypes wiki, incidental)

interface Limits {
    int MAX = 10;
    // @stereotype incidental
    default int doubled() { return MAX * 2; }
}
class Box {
    static final int MAX = 10;
    // @stereotype incidental
    int doubled() { return MAX * 2; }
}

interface Config {
    int MAX = 10;
    // @stereotype incidental
    default int max() { return MAX; }
}
class Bounds {
    static final int MAX = 10;
    // @stereotype incidental
    public int max() { return MAX; }
}
