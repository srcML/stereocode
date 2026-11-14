// The native and the 128-bit integers and Nullable of a primitive type are primitive, and the words required and scoped name no type
// (Method-Stereotypes wiki, get and collaborator)

public class Slot
{
    public required int Index { get; set; }
    private nint handle;
    private nuint size;
    private System.Int128 total;
    private System.Nullable<int> limit;
    private Nullable<double> ratio;

    // @stereotype get
    public int GetIndex() { return Index; }

    // @stereotype get
    public nint Handle() { return handle; }

    // @stereotype get
    public nuint Size() { return size; }

    // @stereotype get
    public System.Int128 Total() { return total; }

    // @stereotype get
    public System.Nullable<int> Limit() { return limit; }

    // @stereotype get
    public Nullable<double> Ratio() { return ratio; }

    // @stereotype set
    public void Take(scoped ref int value) { handle = value; }
}
