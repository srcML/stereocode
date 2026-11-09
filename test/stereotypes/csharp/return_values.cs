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
