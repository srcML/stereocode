// The names in the holes of an interpolated string count as uses of the variables that they name, in every form of the string,
// while doubled braces are text and the alignment and the format of a hole name nothing (Method-Stereotypes wiki, property and get)

public class Account
{
    private string owner = "nobody";
    private decimal balance;
    private int count;
    private int[] limits = new int[2];

    // The example of the issue, which reads the data member as Console.WriteLine(owner) does
    // @stereotype unclassified
    public void PrintOwner()
    {
        Console.WriteLine($"The owner is: {owner}");
    }

    // @stereotype property
    public string Describe() { return $"{owner} has {balance,10:C2}"; }

    // @stereotype property
    public string Folder() { return $@"C:\""{owner}""\"; }

    // @stereotype property
    public string Length() { return @$"{this.owner.Length} characters"; }

    // @stereotype property
    public string Quoted() { return $"""The owner is "{owner}" """; }

    // @stereotype property
    public string Json() { return $$"""{"owner": "{{owner}}"}"""; }

    // @stereotype property
    public string Lines()
    {
        return $"""
            owner {owner}
            done
            """;
    }

    // Doubled braces are text, as is a name in the format of a hole
    // @stereotype incidental
    public string Template() { return $"{{owner}} {42:owner}"; }

    // srcML splits the literal at the quotes inside a hole
    // @stereotype property
    public string Limits() { return $"{string.Join(", ", limits)}"; }

    // @stereotype property
    public string Size() { return $"{(count > 0 ? "some" : "none")}"; }

    // A name after the end of a split string counts, also in the argument that holds the end of the string
    // @stereotype property wrapper
    public string Tail(string[] parts) { return Wrap($"{string.Join(", ", parts)}" + count); }

    private static string Wrap(string text) { return text; }

    // A call in a hole uses its object, while a call of an own method uses no data member
    // @stereotype property
    public string Upper() { return $"{owner.ToUpper()}"; }

    // @stereotype incidental
    public string Again() { return $"{Describe()} {this.Describe()}"; }

    // @stereotype property
    public string Nested() { return $"{$"[{owner}]"}"; }

    // A local or a parameter in a hole is no data member
    // @stereotype incidental
    public string Label(string prefix)
    {
        string suffix = "!";
        return $"{prefix}{suffix}";
    }
}
