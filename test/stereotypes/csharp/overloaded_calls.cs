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

// A call whose number of arguments only a static method of its name takes is a call of that static method, which is a free function
public class Label
{
    private string key;

    public static bool IsData(string key) { return key != null; }

    // @stereotype predicate wrapper
    public bool IsData() { return IsData(key); }

    public static int Width(int margin, int border) { return margin + border; }

    // @stereotype set
    public void Width(int value) { key = "w" + value; }

    // The call with one argument reaches the method
    // @stereotype command
    public void Widen() { Width(4); }

    // @stereotype wrapper stateless
    public int Padded() { return Width(1, 2); }
}

public class ShortLabel : Label
{
    // A static method of a parent is reached as well
    // @stereotype wrapper stateless
    public int Padded(int margin) { return Width(margin, 1) + 1; }
}

// A call whose number of arguments only a method without a definition takes reaches no method with stereotypes and may thus change the object
public abstract class Timer
{
    public abstract int Schedule(int task, long delay, int unit);

    // The call names the abstract method, whose definitions lie in other classes, never the calling method itself
    // @stereotype non-void-command
    public int Schedule(int task) { return Schedule(task, 0L, 1); }

    public static int Width(int margin, int border) { return margin + border; }

    public abstract int Width(string text, string font);

    // The abstract method takes two arguments as the static method does
    // @stereotype non-void-command
    public int Padded() { return Width("a", "b"); }

    // The call without arguments names the method ToString of the class object, which reads the object
    // @stereotype property
    public string ToString(int width) { return ToString() + width; }

    public abstract int Delay(int task);

    // The call names the abstract method, which takes another type than the calling method
    // @stereotype non-void-command
    public int Delay(long task) { return Delay((int)task); }
}

public abstract class Alarm : Timer
{
    private int limit;

    // The abstract method of the parent takes other types than the calling method
    // @stereotype non-void-command
    public int Schedule(string task, long delay, int unit) { return Schedule(0, delay, unit); }

    // The method defines the abstract method of the parent, and its call is a call to itself
    // @stereotype property
    public override int Width(string text, string font) { return text == font ? limit : Width(font, font); }
}

// Where no method of the name takes the arguments of a call, a parent outside the archive may define the called method
public class Names : Roster
{
    // @stereotype non-void-command
    public bool Add(string name) { Add(0, name); return true; }

    // The call without arguments names the query method Count of the parent, which reads the object
    // @stereotype property
    public int Count(int scale) { return Count() * scale; }

    // The parent may define the called method as well, which takes one argument as the calling method does
    // @stereotype command
    public void Append(long count) { Append((int)count); }
}
