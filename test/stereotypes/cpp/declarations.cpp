// Specifiers, initializer lists, and operators of a declaration leave the stereotypes of a method as its body gives them

class Shape {
protected:
    int sides;

public:
    // @stereotype constructor
    explicit Shape(int count) : sides(count) { }

    Shape(const Shape&) = delete;

    // @stereotype get
    virtual int count() const noexcept { return sides; }

    // @stereotype predicate
    bool operator==(const Shape& other) const { return sides == other.sides; }

    // @stereotype property
    double ratio() const { return static_cast<double>(sides) / 2; }

    // @stereotype property
    auto area(int side) const -> int { return side * side * sides / 4; }

    // @stereotype property
    constexpr int doubled() const { return sides * 2; }
};

class Square final : public Shape {
    mutable int reads = 0;

public:
    // @stereotype constructor
    Square() : Shape(4) { }

    // @stereotype get
    int count() const noexcept override { return sides; }

    // A const method that changes a mutable data member is a set
    // @stereotype set
    void touch() const { ++reads; }

    // @stereotype command
    Square& operator+=(int n) { sides += n; reads = 0; return *this; }
};
