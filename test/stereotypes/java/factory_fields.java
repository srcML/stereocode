// A data member created with new and returned makes a factory, also through this. and in parentheses
// (Method-Stereotypes wiki, factory)

class Foo { }
class Cache {
    Foo cache;
    // @stereotype get set factory collaborator
    Foo bare() { cache = new Foo(); return cache; }
    // @stereotype get set factory collaborator
    Foo qualified() { this.cache = new Foo(); return this.cache; }
    // @stereotype factory collaborator stateless
    Foo paren() { return (new Foo()); }
}
