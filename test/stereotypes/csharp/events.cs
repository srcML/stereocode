// A field-like event is a data member of a delegate type, which srcML 1.2 marks as an expression statement or as an event element
// (Method-Stereotypes wiki, Data members and Calls)

using System;
using System.ComponentModel;

public class Model : INotifyPropertyChanged {
    private int level;
    public event PropertyChangedEventHandler PropertyChanged;
    public event EventHandler Closed = delegate { };
    [field: NonSerialized] public event EventHandler Saved;
    internal static event Action<int> Tick;

    // A call of an event reads the event and is a call of a free function, also in a class with a parent outside the archive
    // @stereotype wrapper
    public void Raise(string name) { PropertyChanged(this, new PropertyChangedEventArgs(name)); }

    // @stereotype wrapper
    public void RaiseIfSet(string name) { PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name)); }

    // @stereotype wrapper
    public void Save() { Saved(this, EventArgs.Empty); }

    // No call changes the delegate that an event holds, so a call of any method on an event is such a call as well
    // @stereotype wrapper
    public void SaveSafely() { Saved.SafeInvoke(this, EventArgs.Empty); }

    // The delegate type of an event is a type of another class
    // @stereotype predicate collaborator
    public bool HasListeners() { return Closed != null; }

    // A handler that a method adds changes the event
    // @stereotype set collaborator
    public void Listen(EventHandler handler) { Closed += handler; }

    // A static event is a global, which is no state of the object
    // @stereotype collaborator incidental
    public void Watch(Action<int> handler) { Tick += handler; }

    // @stereotype wrapper
    public void Beat() { Tick(level); }
}

// An event with accessors is no data member
public class Relay {
    private EventHandler inner;

    public event EventHandler Changed { add { inner += value; } remove { inner -= value; } }
}

public class Panel {
    public event EventHandler Opened;

    // @stereotype set collaborator
    public void Forget(EventHandler handler) { Opened -= handler; }
}
