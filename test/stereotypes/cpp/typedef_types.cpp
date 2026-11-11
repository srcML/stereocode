// A class, struct, or union without a name that a typedef names is a type of that name, whose methods get stereotypes
// (Class-Stereotypes wiki, and Method-Stereotypes wiki, get, set, and property)

// @stereotype data-class small-class
typedef class {
    int value;
public:
    // @stereotype get
    int getValue() const { return value; }
    // @stereotype set
    void setValue(int v) { value = v; }
} Counter;

// The first name of the typedef that names the type itself, Point and not the pointer PointPtr, names the type
// @stereotype data-provider data-struct small-struct
typedef struct {
    int x;
    int y;
    // @stereotype get
    int getX() const { return x; }
} *PointPtr, Point;

// A type that inherits from a type of a typedef reaches its data members
// @stereotype data-provider small-struct
struct Pixel : public Point {
    // @stereotype property
    int sum() const { return x + y; }
};

// A union of a typedef, as unions are with -u
// @stereotype data-provider data-union small-union
typedef union {
    int i;
    float f;
    // @stereotype get
    int asInt() const { return i; }
} Number;

// A typedef in a type names a nested type, whose accessor a call on a data member of the type reaches
// @stereotype boundary data-provider small-struct
struct Holder {
    // @stereotype data-provider data-struct small-struct
    typedef struct {
        int depth;
        // @stereotype get
        int getDepth() const { return depth; }
    } Level;
    Level level;
    // @stereotype property collaborator
    int depth() { return level.getDepth(); }
};

// A type nested in a type of a typedef takes the name of the typedef as its qualifier
// @stereotype boundary data-provider small-struct
typedef struct {
    // @stereotype data-provider data-struct small-struct
    struct Leaf {
        int weight;
        // @stereotype get
        int getWeight() const { return weight; }
    } leaf;
    // @stereotype property collaborator
    int weight() { return leaf.getWeight(); }
} Tree;
