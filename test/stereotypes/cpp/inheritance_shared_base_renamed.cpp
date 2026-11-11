// The hierarchy of inheritance_shared_base.cpp under other names, whose order of the types hides the defect

class B { protected: int b; public: int getB() const { return b; } };
class C : public B { public: void c1() {} };
// @stereotype commander data-class small-class
class D : public C { public:
    // @stereotype set
    void setB(int v) { b = v; } };
class X : public B, public D { public: void x1() {} };
