// Inherited data members do not depend on the order of the types (Method-Stereotypes wiki, set):
// b reaches Leaf through Mid, while Top inherits Base twice

class Base { protected: int b; public: int getB() const { return b; } };
class Mid : public Base { public: void c1() {} };
// @stereotype commander data-class small-class
class Leaf : public Mid { public:
    // @stereotype set
    void setB(int v) { b = v; } };
class Top : public Base, public Leaf { public: void x1() {} };
