// A using directive names the namespace of a name that several types have, here the class Tag beside the class Tag in the class Query.Evaluator

namespace Model
{
    public class Tag
    {
        private string name;

        // @stereotype get
        public string GetName() { return name; }
    }
}
