// A call to an inherited method counts by the stereotypes of that method, also through base

public class Sensor
{
    protected double reference;

    // @stereotype get
    public double Reference() { return reference; }

    // @stereotype incidental
    public int Version() { return 2; }

    // @stereotype set
    public void Calibrate(double value) { reference = value; }
}

public class Probe : Sensor
{
    private double reading;

    // @stereotype property
    public double Offset() { return reading - Reference(); }

    // @stereotype property
    public double Scaled() { return base.Reference() * 2; }

    // @stereotype wrapper stateless
    public int DoubledVersion() { return Version() * 2; }

    // A call to an inherited set may change the object
    // @stereotype command
    public void Reset() { Calibrate(0); }
}

public class Counter
{
    protected int count;
    protected int reads;

    // @stereotype get set
    public virtual int Count() { reads++; return count; }
}

public class QuietCounter : Counter
{
    // A method with the signature of a method of the parent replaces that method in the calls of the child
    // @stereotype get
    public override int Count() { return count; }

    // @stereotype property
    public int Doubled() { return Count() * 2; }

    // A call through base reaches the method of the parent
    // @stereotype property non-void-command
    public int Counted() { return base.Count() * 2; }
}
