// Method-Stereotypes wiki, Creational, C# example (PowerShell) in a minimal class

public class LocalGroup
{
    private string name;

    // @stereotype copy-constructor
    public LocalGroup(LocalGroup other)
    {
        name = other.name;
    }

    // Passing this to a constructor reads no state of the object
    // @stereotype factory stateless
    public LocalGroup Clone()
    {
        return new LocalGroup(this);
    }
}
