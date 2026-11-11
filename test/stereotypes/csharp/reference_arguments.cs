// A variable passed with ref or out may be written by the called method, therefore the call counts as a write of the variable,
// as for a field, a ref parameter, or an out parameter, while a value parameter passed with ref changes only its own copy

// @stereotype data-class
public class Counter
{
    private int count;
    private int last;

    // @stereotype set
    public void Increment()
    {
        System.Threading.Interlocked.Increment(ref count);
    }

    // @stereotype set
    public void Read(string text)
    {
        Parser.TryRead(text, out last);
    }

    // @stereotype get
    public int Count()
    {
        return count;
    }
}

public static class NativeFiles
{
    // @stereotype command
    public static void Fill(ref FindData data)
    {
        Native.Read(ref data);
    }

    // @stereotype command
    public static void Open(string path, out FindData data)
    {
        Native.Open(path, out data);
    }

    // @stereotype wrapper
    public static void Copy(int size)
    {
        Native.Use(ref size);
    }
}
