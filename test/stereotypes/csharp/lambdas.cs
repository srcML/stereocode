// Returns and locals inside lambdas belong to the lambda (Method-Stereotypes wiki, get and controller)

using System;

// @stereotype small-class
class Clock {
    private long now;
    // @stereotype controller
    public void Register(Registry r) { r.Add(() => { return now; }); }
    // @stereotype set
    public void Tick() { now++; }
}

public class Holder
{
    private int value;

    // @stereotype controller
    public void Register(Registry r)
    {
        r.Add(() => { return value; });
    }
}

// The local total of the lambda is no data member, and the static total is no object state
public class Calc {
    static int total = 0;
    private Func<int, int> op = x => { int total = x * 2; return total; };
    // @stereotype incidental
    public int Peek() { return total; }
}
