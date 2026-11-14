using Model;

namespace View
{
    public class Element
    {
        private Tag tag;

        // The call reaches the get of the class in the namespace of the using directive
        // @stereotype property collaborator
        public string TagName() { return tag.GetName(); }
    }
}
