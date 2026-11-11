// Class signatures count methods, once each (Class-Stereotypes wiki, rules for class stereotype identification)

// One build compiles one branch of #if and #else, so Boost counts once
// @stereotype data-class small-class
public class Engine
{
    private int rpm;

    public int Speed() { return rpm; }
#if FAST
    public void Boost() { rpm = 9000; }
#else
    public void Boost() { rpm = 5000; }
#endif
}

// The get accessors of three properties are three methods, although each accessor is named get
// @stereotype data-provider data-class
public class Gauge
{
    private int level;
    private int limit;
    private int step;

    public int Level { get { return level; } }
    public int Limit { get { return limit; } }
    public int Step { get { return step; } }
}

// One build compiles one branch of #if and #else, so each accessor of Rate counts once, keyed by its property and its name
// @stereotype data-class small-class
public class Meter
{
    private int rate;

#if FAST
    public int Rate { get { return rate; } set { rate = value * 2; } }
#else
    public int Rate { get { return rate; } set { rate = value; } }
#endif
}

// Indexers with different parameters are different members, so their accessors count apart
// @stereotype data-provider data-class
public class Grid
{
    private int cell;

    public int Size() { return cell; }
    public int this[int i] { get { return cell; } }
    public int this[string key] { get { return cell; } }
}
