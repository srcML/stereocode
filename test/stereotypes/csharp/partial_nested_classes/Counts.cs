// The parts of a partial type nested in a partial type form one type under its qualified name

public partial class Outer
{
    public partial class Part
    {
        private int count;
        // @stereotype get
        public int Count() { return count; }
    }
}
