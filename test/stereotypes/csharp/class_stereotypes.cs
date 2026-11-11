// Class stereotypes (Class-Stereotypes wiki, rules for class stereotype identification)
// The method labels are listed to show the class signature; constructors do not count

// @stereotype entity boundary
public class Measure
{
    private int value;
    private int changes;

    // @stereotype property collaborator
    public int Scaled(Scale scale) { return value * 2; }

    // @stereotype command collaborator
    public void Adjust(Scale scale) { value *= 2; changes++; }

    // @stereotype get
    public int Value() { return value; }
}

// @stereotype minimal-entity boundary
public class Gauge
{
    private int level;
    private int limit;

    // @stereotype get collaborator
    public int Level(Scale scale) { return level; }

    // @stereotype set collaborator
    public void SetLevel(int amount, Scale scale) { level = amount; }

    // @stereotype command
    public void Fill() { level = limit; limit++; }
}

// @stereotype data-provider
public class Circle
{
    private double radius;

    // @stereotype get
    public double Radius() { return radius; }

    // @stereotype predicate
    public bool IsUnit() { return radius == 1.0; }

    // @stereotype property
    public double Area() { return 3.14159 * radius * radius; }

    // @stereotype set
    public void SetRadius(double r) { radius = r; }
}

// @stereotype minimal-entity commander
public class Counter
{
    private int count;
    private int steps;

    // @stereotype set
    public void SetCount(int c) { count = c; }

    // @stereotype command
    public void Increment() { count++; steps++; }

    // @stereotype command
    public void Decrement() { count--; steps++; }

    // @stereotype get
    public int Count() { return count; }
}

// @stereotype boundary
public class Exporter
{
    private int pages;

    // @stereotype get collaborator
    public int Pages(Printer printer) { return pages; }

    // @stereotype property collaborator
    public int Sheets(Printer printer) { return pages / 2; }

    // @stereotype set
    public void SetPages(int p) { pages = p; }
}

// @stereotype factory controller
public class WidgetFactory
{
    private int style;

    // @stereotype property factory collaborator
    public Widget Plain() { Widget widget = new Widget(style); return widget; }

    // @stereotype property factory collaborator
    public Widget Bold() { Widget widget = new Widget(style + 1); return widget; }

    // @stereotype property factory collaborator
    public Widget Italic() { Widget widget = new Widget(style + 2); return widget; }
}

// @stereotype controller
public class Spooler
{
    private int pages;

    // @stereotype controller
    public void SendFirst(Printer printer) { printer.Print(pages); }

    // @stereotype controller
    public void SendSecond(Printer printer) { printer.Print(pages + 1); }

    // @stereotype controller
    public void SendThird(Printer printer) { printer.Print(pages + 2); }

    // @stereotype get
    public int Pages() { return pages; }
}

// @stereotype pure-controller
public class Dispatcher
{
    private int pages;

    // @stereotype controller
    public void SendFirst(Printer printer) { printer.Print(pages); }

    // @stereotype controller
    public void SendSecond(Printer printer) { printer.Print(pages + 1); }

    // @stereotype controller
    public void SendThird(Printer printer) { printer.Print(pages + 2); }
}

// @stereotype lazy-class
public class Settings
{
    private int width;
    private int height;

    // @stereotype get
    public int Width() { return width; }

    // @stereotype get
    public int Height() { return height; }

    // @stereotype set
    public void SetWidth(int w) { width = w; }

    // @stereotype incidental
    public int Version() { return 2; }

    // @stereotype empty
    public void Reload() { }
}

// @stereotype entity degenerate
public class Plugin
{
    private int state;
    private int loads;

    // @stereotype incidental
    public int Version() { return 1; }

    // @stereotype incidental
    public string Name() { return "plugin"; }

    // @stereotype empty
    public void Unload() { }

    // @stereotype property
    public int State() { return state + 1; }

    // @stereotype command
    public void Reset() { state = 0; loads = 0; }
}

// @stereotype data-class
public class Pair
{
    private int first;
    private int second;

    // @stereotype get
    public int First() { return first; }

    // @stereotype get
    public int Second() { return second; }

    // @stereotype set
    public void SetFirst(int f) { first = f; }
}

// @stereotype entity small-class
public class Tank
{
    private int level;
    private int capacity;

    // @stereotype property
    public int Percent() { return level * 100 / capacity; }

    // @stereotype command
    public void Fill() { level = capacity; capacity++; }
}

// @stereotype empty
public class Tag
{
    private int id;

    // @stereotype constructor
    public Tag() { id = 0; }
}
