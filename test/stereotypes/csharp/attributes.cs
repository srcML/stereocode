// An attribute names neither the type of a variable nor a variable, and it calls no function, whatever arguments the attribute takes

using System;
using System.Diagnostics;

class Meter {
    private int value;
    private string name;

    // @stereotype set
    void Set([Range(Minimum = 0, Maximum = 150)] int value) { this.value = value; }

    // @stereotype set
    void Rename([NotNull] string name) { this.name = name; }

    // @stereotype get
    [return: NotNull] string Name() { return name; }

    // @stereotype get
    [Obsolete("Use Name")] int Value() { return value; }

    // The local function is code of the method, and its attribute calls no function
    // @stereotype incidental
    int Limit() {
        [Obsolete(DiagnosticId = "X1")] int Twice(int count) { return count * 2; }
        return Twice(2);
    }

    // @stereotype property
    int Scaled() {
        [Conditional("DEBUG")] static int Scale(int count) { return count * 10; }
        return Scale(value);
    }
}
