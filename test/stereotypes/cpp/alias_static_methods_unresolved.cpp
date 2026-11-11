// An alias of a pointer after const, a template alias, which may name no single type, and a cycle of aliases rename no type, so the match
// of a static method with its definition keeps their names as written, which here tell each static method from its non-static overload

typedef int* CountPtr;
template <typename T> using Ptr = T*;
typedef Ring Loop;
typedef Loop Ring;

// const CountPtr is a constant pointer to int, unlike const int*, a pointer to a constant int
// @stereotype data-provider data-class small-class
class Pointers {
    int x;
public:
    static bool first(const CountPtr p);
    int first(const int* c) const;
};
// @stereotype predicate
bool Pointers::first(const CountPtr p) { return p != 0; }
// @stereotype get
int Pointers::first(const int* c) const { return x; }

// A parameter of a template alias is a type that is no primitive, so the non-static overload is a collaborator
// @stereotype data-provider boundary data-class small-class
class Templates {
    int x;
public:
    static bool empty(Ptr<int> p);
    int empty(Ptr<char> p) const;
};
// @stereotype predicate
bool Templates::empty(Ptr<int> p) { return p == 0; }
// @stereotype get collaborator
int Templates::empty(Ptr<char> p) const { return x; }

// The two names of a cycle of aliases, which name no type, so a parameter of either name is a type that is no primitive too
// @stereotype data-provider boundary data-class small-class
class Wheel {
    int x;
public:
    static bool spin(Loop l);
    int spin(Ring r) const;
};
// @stereotype predicate
bool Wheel::spin(Loop l) { return l == 0; }
// @stereotype get collaborator
int Wheel::spin(Ring r) const { return x; }
