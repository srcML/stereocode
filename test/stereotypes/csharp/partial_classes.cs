// The parts of a partial class form one class (Method-Stereotypes wiki, command)

// @stereotype commander small-class
public abstract partial class Job {
    private int runs;
    // Run is a method of the class, declared in the other part
    // @stereotype command
    public void Execute() { runs++; Run(); }
}
public abstract partial class Job {
    protected abstract void Run();
}

// @stereotype commander small-class
public abstract class Chore {
    private int runs;
    // @stereotype command
    public void Execute() { runs++; Run(); }
    protected abstract void Run();
}
