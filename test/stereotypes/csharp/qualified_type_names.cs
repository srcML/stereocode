// A qualified type name that ends with the name of the own class names another class, unless its qualifiers are those of the own class

namespace Wire
{
    public class Request { }
}

namespace Http
{
    public class Request
    {
        private int size;

        // @stereotype set collaborator
        public void Copy(Wire.Request other) { size = 1; }

        // @stereotype set
        public void Take(Http.Request other) { size = 2; }

        // @stereotype set
        public void Keep(Request other) { size = 3; }
    }
}
