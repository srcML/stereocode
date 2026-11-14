// A static import of a type outside the archive may name any function, so a call without an object stays a call of a free function,
// also in a class with a parent outside the archive (Method-Stereotypes wiki, Calls)
using System.Collections.ObjectModel;
using static System.Math;

class Angles : Collection<double>
{
    // @stereotype property wrapper
    double Largest() { return Max(this[0], this[1]); }

    // A call through the object names a method of the object
    // @stereotype command
    void Reset() { this.Clear(); }

    // @stereotype predicate
    bool Has(double angle) { return this.Contains(angle); }
}
