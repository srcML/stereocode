// An attribute names no variable and calls no function, whatever arguments the attribute takes

class Meter {
    int value;
public:
    // @stereotype incidental
    int limit() {
        [[maybe_unused]] int count = 2;
        [[deprecated("old")]] int other = 3;
        return count;
    }

    // @stereotype property
    int twice() {
        if (value > 0) [[likely]] { return value * 2; }
        return 0;
    }

    // @stereotype get
    [[nodiscard]] int get() const { return value; }

    // @stereotype get
    [[deprecated("use get")]] int read() const { return value; }

    // @stereotype set
    void set([[maybe_unused]] int next) { value = next; }
};
