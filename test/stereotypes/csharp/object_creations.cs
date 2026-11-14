// A creation without an argument list after its type, as the target-typed new(), the initializer of new Report { }, and the anonymous type of
// new { Total = 1 }, is a constructor call, while an array creation calls no constructor

public class Report
{
    private int pages;

    // @stereotype factory stateless
    public Report Blank() { return new(); }

    // @stereotype factory stateless
    public Report Sized(int count) { return new(count); }

    // @stereotype factory stateless
    public Report Empty() { return new Report { }; }

    // srcML marks the type and the arguments before an initializer, as Report(count) of new Report(count) { }, as a macro
    // @stereotype factory stateless
    public Report Counted(int count) { return new Report(count) { }; }

    // @stereotype factory collaborator stateless
    public object Summary() { return new { Total = 1 }; }

    // @stereotype incidental
    public int[] Numbers() { return new int[3]; }

    // @stereotype incidental
    public int[] Listed() { return new[] { 1, 2 }; }

    // @stereotype factory incidental
    public Report[] Shelf() { return new Report[2]; }
}
