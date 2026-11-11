// C# assignments: ??=, a change inside a declaration, and object initializers (Method-Stereotypes wiki, set)

class Foo { public int P; }
class Registry { int P; Foo o; Helper h; int seed;
    // The initializer member P belongs to Foo, not to Registry
    // @stereotype set collaborator
    void Assign() { o = new Foo { P = 1 }; }
    // @stereotype set collaborator
    void Coalesce() { h ??= new Helper(); }
    // @stereotype property set
    int NextId() { int id = seed++; return id; }
}
