public partial class Outer
{
    public partial class Part
    {
        // @stereotype set
        public void Reset() { count = 0; }
    }
}

// A partial type of the same name in another type is another type
public partial class Other
{
    public partial class Part
    {
        private int total;
        // @stereotype incidental
        public void Reset() { count = 0; }
    }
}
