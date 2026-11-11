// Inherited data members do not depend on the order of the types (Method-Stereotypes wiki, get):
// a reaches Top through Right, which Bottom also inherits

class Base { protected: int a; };
class Left : public Base { };
class Right : public Base { };
class Bottom : public Left, public Right { };
class Top : public Right { public:
    // @stereotype get
    int getA() const { return a; } };
