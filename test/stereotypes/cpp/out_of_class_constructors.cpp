// Constructors, destructors, and methods defined outside their class (Method-Stereotypes wiki, Creational)

class Foo {
  int m_a;
public:
  Foo(int a);
  Foo(const Foo& other);
  ~Foo();
};

// @stereotype constructor
Foo::Foo(int a) : m_a(a) {}
// @stereotype copy-constructor
Foo::Foo(const Foo& other) : m_a(other.m_a) {}
// @stereotype destructor
Foo::~Foo() {}

class Account {
    int balance;
public:
    Account();
    Account(const Account& other);
    ~Account();
    int get() const;
};

// @stereotype constructor
Account::Account() { balance = 0; }
// @stereotype copy-constructor
Account::Account(const Account& other) { balance = other.balance; }
// @stereotype destructor
Account::~Account() { }
// @stereotype get
int Account::get() const { return balance; }

class A {
    int x;
    int y;
public:
    A();
    A(const A& other);
    ~A();
    int getX() const;
    A* clone() const;
    // Passing *this to a constructor reads no state of the object
    // @stereotype factory stateless
    A* cloneIn() const { return new A(*this); }
};

// @stereotype constructor
A::A() : x(0), y(0) {}
// @stereotype copy-constructor
A::A(const A& other) : x(other.x), y(other.y) {}
// @stereotype destructor
A::~A() {}
// @stereotype get
int A::getX() const { return x; }
// Labeled like cloneIn
// @stereotype factory stateless
A* A::clone() const { return new A(*this); }
