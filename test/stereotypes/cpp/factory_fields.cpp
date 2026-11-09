// A data member created with new and returned makes a factory, also through this->
// (Method-Stereotypes wiki, factory)

class Foo { };
class A {
    Foo* cache;
public:
    // @stereotype get set factory collaborator
    Foo* bare() { cache = new Foo(); return cache; }
    // @stereotype get set factory collaborator
    Foo* qualified() { this->cache = new Foo(); return this->cache; }
};
