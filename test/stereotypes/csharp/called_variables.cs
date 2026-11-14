// A call of a variable runs the function that the variable holds, by its name or by Invoke, so the call reads the variable and stays a call
// of a function (Method-Stereotypes wiki, Calls; Free-Function-Stereotypes wiki, literal and wrapper)

using System;

public class Button {
    private Action onFire;
    private Func<int, bool> filter;
    private int count;

    // A call of a data member reads the data member, so the method uses the state of the object
    // @stereotype wrapper
    public void Fire() { onFire(); }

    // @stereotype wrapper
    public void FireLong() { onFire.Invoke(); }

    // @stereotype wrapper
    public void FireIfSet() { onFire?.Invoke(); }

    // @stereotype wrapper
    public void FireThroughObject() { this.onFire(); }

    // @stereotype set collaborator
    public void Press(int code) { if (filter(code)) count++; }

    // @stereotype wrapper
    public void Each(Action<int> visit) { visit(count); }

    // @stereotype get collaborator
    public Action Handler() { return onFire; }

    // @stereotype wrapper
    public void Start() { Action held = Handler(); held(); }

    // A local that a lambda initializes holds code of the method itself, so its call is no call
    // @stereotype property collaborator
    public int Twice() { Func<int, int> doubled = v => 2 * v; return doubled(count); }

    // The code of a local function is code of the method around it, and its call is no call
    // @stereotype set
    public void Bump() { void Step() { count++; } Step(); }

    // @stereotype property
    public int Next() { int Add(int step) { return count + step; } return Add(1); }

    // @stereotype incidental
    public int Seven() { int Add(int left, int right) { return left + right; } return Add(3, 4); }
}

public static class Runner {
    // A call of a parameter reads the parameter, so the function is no literal, and changes no object of the caller
    // @stereotype wrapper
    public static void Run(Action action) { action(); }

    // @stereotype wrapper
    public static void RunLong(Action action) { action.Invoke(); }

    // @stereotype property
    public static int Compute(int x) { int Square(int v) { return v * v; } return Square(x) + 1; }

    // @stereotype property
    public static int Twice(int x) { Func<int, int> doubled = v => 2 * v; return doubled(x); }
}

public class Job {
    private int runs;

    // @stereotype set
    public void Invoke() { runs++; }
}

public class Scheduler {
    private Job last;

    // Invoke of a class of the archive is a method of that class, so its call on a parameter or a local is a call on another object
    // @stereotype controller stateless
    public void Start(Job job) { job.Invoke(); }

    // @stereotype controller stateless
    public void StartNew() { Job job = new Job(); job.Invoke(); }

    // @stereotype command collaborator
    public void Repeat() { last.Invoke(); }

    // @stereotype command
    public static void StartNow(Job job) { job.Invoke(); }

    // srcML marks a method whose parameter has the default literal as a declaration, which is no data member that a call reads
    // @stereotype wrapper stateless
    public int Seven() { Reset(); return 7; }

    private void Reset(Job job = default) { last = job; }
}
