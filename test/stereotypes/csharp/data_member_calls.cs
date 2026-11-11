// A call on a data member whose type is a type of the archive counts by the stereotypes of the called method of that type:
// a call to an accessor reads the data member, a call to a degenerate method counts like a call of a free function,
// and a call to any other method may change the data member

public class Engine
{
    private int rpm;

    // @stereotype get
    public int Rpm() { return rpm; }

    // @stereotype set
    public void Start() { rpm = 800; }

    // @stereotype wrapper stateless
    public void Log() { Trace.Write("engine"); }
}

public interface IGauge
{
    int Read();
}

public class Car
{
    private Engine engine;
    private IGauge gauge;

    public Engine Spare { get; set; }

    // @stereotype property collaborator
    public int Speed() { return engine.Rpm() / 100; }

    // A property whose type is a type of the archive counts like a data member
    // @stereotype property collaborator
    public int SpareSpeed() { return Spare.Rpm() / 100; }

    // @stereotype property collaborator
    public int OwnSpeed() { return this.engine.Rpm() / 100; }

    // @stereotype command collaborator
    public void Go() { engine.Start(); }

    // @stereotype wrapper
    public void Report() { engine.Log(); }

    // A method that the type of the data member declares without a body has no stereotypes and may change the data member
    // @stereotype property non-void-command collaborator
    public int Level() { return gauge.Read(); }
}
