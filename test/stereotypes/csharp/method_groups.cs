// A method group, the name of a method of the own type passed as a delegate without a call, as OnWorkFinished of Start(OnWorkFinished),
// is no call, no use of the state of the object, and no unknown variable

public class Worker
{
    // @stereotype wrapper stateless
    public void Start(Action onFinished)
    {
        Log.Write("Working...");
        onFinished();
    }
}

// Methods that pass OnWorkFinished to another object as a delegate
public class Demo
{
    private int count;
    private Worker worker;

    // @stereotype controller stateless
    public void Run()
    {
        var local = new Worker();
        local.Start(OnWorkFinished);
    }

    // @stereotype controller stateless
    public void RunSelf()
    {
        var local = new Worker();
        local.Start(this.OnWorkFinished);
    }

    // @stereotype collaborator incidental
    public Action Handler() { return OnWorkFinished; }

    // @stereotype controller incidental
    public void Hook(Worker other) { other.Finished += this.OnWorkFinished; }

    // A data member that a call of the method passes along is a use of the state
    // @stereotype controller
    public void Pass(Worker other) { other.Start(this.OnWorkFinished, count); }

    // Start of Worker is a degenerate method, whose call on the data member counts like a call of a free function
    // @stereotype wrapper
    public void RunField() { worker.Start(OnWorkFinished); }

    // @stereotype set
    private void OnWorkFinished() { count++; }
}

// A parent outside the archive may declare any name, and a method group still names no variable
public class Form1 : Form
{
    // @stereotype controller stateless
    public void Wire(Button button) { button.Click(OnClick); }

    // @stereotype empty
    private void OnClick() { }
}
