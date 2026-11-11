// Structures other than classes (run with -t for structs and -u for unions): the class rules name
// the structure, as in data-struct and small-union

// @stereotype data-provider data-struct small-struct
struct Point {
    int x;
    int y;
    // @stereotype get
    int getX() const { return x; }
};

// @stereotype empty
struct Tag {
    int id;
};

// @stereotype data-provider data-union small-union
union Number {
    int   asInt;
    float asFloat;
    // @stereotype get
    int integer() const { return asInt; }
};
