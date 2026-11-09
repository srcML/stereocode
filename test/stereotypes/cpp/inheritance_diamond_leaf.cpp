// The hierarchy of inheritance_diamond_top.cpp with the leaf named Leaf, whose order of the types hides the defect

class Base { protected: int a; };
class Left : public Base { };
class Right : public Base { };
class Bottom : public Left, public Right { };
class Leaf : public Right { public:
    // @stereotype get
    int getA() const { return a; } };
