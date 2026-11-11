// Calls to overloaded methods of the own type: a call reaches the overloads of its name that take as many arguments as the call passes

public class Gauge
{
    private double level;
    private int offset;

    // @stereotype get
    public double Level() { return level; }

    // @stereotype set
    public void Level(double value) { level = value; }

    // The call without arguments reaches the get alone, while the set takes one argument
    // @stereotype property
    public double Doubled() { return Level() * 2; }

    // @stereotype property
    public int Scale(int unit) { return offset * unit; }

    // @stereotype set
    public void Scale(double factor) { level = level * factor; }

    // Both overloads take one argument, and the set may change the object
    // @stereotype property non-void-command
    public int Scaled() { return Scale(2) + 1; }

    // @stereotype property
    public int Offset(int unit = 1) { return offset * unit; }

    // @stereotype set
    public void Offset(int low, int high) { offset = high - low; }

    // The optional parameter lets the call without arguments reach the property alone
    // @stereotype property
    public int Shifted() { return Offset() + 1; }

    // @stereotype get
    public int Total(params int[] values) { return offset; }

    // @stereotype set
    public void Total(int first, int second) { offset = first + second; }

    // A call with three arguments reaches the overload with params alone
    // @stereotype property
    public int Summed() { return Total(1, 2, 3) + 1; }
}
