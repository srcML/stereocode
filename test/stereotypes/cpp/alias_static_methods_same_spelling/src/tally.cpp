// A class with the name of an alias of a header

namespace tally {
    // @stereotype data-provider data-class small-class
    class Count {
        int value;
    public:
        // @stereotype get
        int get() const { return value; }
    };
}
