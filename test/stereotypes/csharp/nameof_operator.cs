// The operator nameof takes the name of a variable and reads no value of the variable

public class Field
{
    private string label;

    // @stereotype incidental
    public string LabelName() { return nameof(label); }

    // @stereotype incidental
    public string QualifiedName() { return nameof(this.label); }

    // @stereotype property
    public string Described() { return nameof(label) + label; }
}
