// The operator ! after a value says that the value is not null and changes no variable that the code names

class Node {
    public string Name;
    private int size;

    // @stereotype set
    public void Clear() { size = 0; }
}

class Forgiving {
    private int[] buffer;
    private int count;
    private Node node;
    private Node[] nodes;
    private System.IO.TextWriter writer;
    private string name;

    // @stereotype set
    void Store(int value) { buffer![0] = value; }

    // @stereotype command
    void Fill(int value) { buffer![count] = value; count = 0; }

    // @stereotype set collaborator
    void Rename(string name) { node!.Name = name; }

    // A call on a data member of a type outside the archive may change the data member
    // @stereotype command collaborator
    void Flush() { writer!.Flush(); }

    // @stereotype command collaborator
    void Clear() { node!.Clear(); }

    // @stereotype command collaborator
    void ClearFirst() { nodes![0]!.Clear(); }

    // @stereotype get
    string Name() { return name!; }

    // @stereotype property collaborator
    int Length() { return node!.Name!.Length; }

    // A call on a parameter is a call on another object
    // @stereotype controller stateless
    void Reset(Node other) { other!.Clear(); }

    // @stereotype controller stateless
    void Trim(Node other) { other.Name!.Trim(); }
}
