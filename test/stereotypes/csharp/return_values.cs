// Return types and return expressions (Method-Stereotypes wiki, get, predicate, and property)

class Samples {
    int[] arr;
    int x;
    // @stereotype property
    int SumAt(int i) { return arr[i] + x; }
    // @stereotype predicate
    bool IsZero(int i) { return arr[i] == 0; }
    // @stereotype get
    int At(int i) { return arr[i]; }
    // A method that returns a reference to a data member returns the data member
    // @stereotype get
    ref int Total() { return ref x; }
    // @stereotype get
    ref int Slot(int i) { return ref arr[i]; }
    // @stereotype get
    ref int Own() { return ref this.x; }
}

public class Flags
{
    private int count;

    // @stereotype predicate
    public System.Boolean IsEmptyQualified()
    {
        return count == 0;
    }

    // @stereotype predicate
    public bool IsEmpty()
    {
        return count == 0;
    }
}

// A container of Boolean values returns the container, which is no Boolean, while a nullable Boolean is one
public class Switches
{
    private List<bool> states;
    private bool? ready;
    private int count;

    // @stereotype property
    public List<bool> Active() { return count > 0 ? states : null; }

    // @stereotype predicate
    public bool? Ready() { return count > 0 ? ready : null; }
}
