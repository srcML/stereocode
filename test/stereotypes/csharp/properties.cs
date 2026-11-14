// Property accessors, expression-bodied members, event accessors, and types nested in records
// (Method-Stereotypes wiki, get and set)

class Connection {
    string _server;
    public string Server
    {
        // @stereotype get
        get { return _server; }
        // value is the implicit parameter of the setter
        // @stereotype set
        set
        {
            if (value == null) value = string.Empty;
            _server = value;
        }
    }
}

public class Grid {
    private int[] cells = new int[10];
    private int count;
    public int this[int i] {
        // @stereotype get
        get { return cells[i]; }
        // @stereotype set
        set { cells[i] = value; } }
    // @stereotype get @defect srcML marks an expression-bodied property as an expression (#70)
    public int Count => count;
    public int Size {
        // @stereotype get @defect srcML marks an expression-bodied accessor as an expression (#70)
        get => count;
        // @stereotype set @defect srcML marks an expression-bodied accessor as an expression (#70)
        set => count = value; }
    public event System.EventHandler Changed { add { count++; } remove { count--; } }
    // @stereotype get @defect srcML marks an expression-bodied method as an expression (#76)
    public int GetCount() => count;
}

// srcML 1.2 marks expression-bodied members as expression statements
public class Account {
    private int balance;
    // @stereotype get @defect srcML marks an expression-bodied method as an expression (#76)
    public int Get() => balance;
    // @stereotype predicate @defect srcML marks an expression-bodied method as an expression (#76)
    public bool IsEmpty() => balance == 0;
    // @stereotype get @defect srcML marks an expression-bodied property as an expression (#70)
    public int Balance => balance;
    // @stereotype get
    public int Plain() { return balance; }
}

// An event with add and remove accessors hides the member after it in srcML 1.2
public class Acc {
    private int balance;
    public event EventHandler Changed { add { h += value; } remove { h -= value; } }
    // @stereotype get @defect srcML hides the member after an event with accessors
    public int Get() { return balance; }
    // @stereotype set
    public void Set(int b) { balance = b; }
}

// srcML marks the record as a function, and the methods of the class inside it keep their labels in the XML output
public record Person(string Name) {
    public class Builder {
        private string name;
        // @stereotype get
        public string Get() { return name; }
        // @stereotype set
        public void Set(string n) { name = n; }
    }
}
