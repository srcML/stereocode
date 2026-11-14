// Calls to the methods of the own type: a call to a method labeled as an accessor reads the object,
// a call to a degenerate method neither reads nor changes the object, and a call to any other method of the type may change the object

public class Thermometer
{
    private double degrees;

    // @stereotype get
    public double Celsius() { return degrees; }

    // @stereotype property
    public double Fahrenheit() { return Celsius() * 9 / 5 + 32; }

    // A call to a method that reads through its own calls reads the object as well
    // @stereotype property
    public double Offset() { return Fahrenheit() - 32; }

    // @stereotype predicate
    public bool Freezing() { return this.Celsius() <= 0; }

    // @stereotype empty
    public void Check() { }

    // A call to a degenerate method counts like a call of a free function
    // @stereotype property wrapper
    public double Checked() { Check(); return degrees * 2; }

    // @stereotype set
    public void Reset() { degrees = 0; }

    // A call to a set may change the object
    // @stereotype command
    public void Restart() { Reset(); }
}

// A call that reaches the calling method itself counts by the stereotypes that the method has without the call
public class Limit
{
    private int limit;

    // @stereotype predicate
    public bool Accepts(int value) { return value < limit; }

    // The call with one argument reaches the two overloads, of which the other one only reads
    // @stereotype predicate
    public bool Accepts(long value) { return Accepts((int)value); }

    // @stereotype property
    public int Next(int level) { return level > limit ? level : Next(level + 1); }

    // A call of a method that changes the object to itself may change the object
    // @stereotype command
    public void Drain(int steps) { if (steps > 0) { limit--; Drain(steps - 1); } }
}
