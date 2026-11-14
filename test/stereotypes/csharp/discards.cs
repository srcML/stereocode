// The discard _ takes a value and names no variable

class Job : System.IDisposable {
    private int count;

    // A parent outside the archive declares no data member of the name _
    // @stereotype wrapper stateless
    void Start() { _ = System.Threading.Tasks.Task.Run(() => 1); }

    // @stereotype predicate wrapper
    bool Reads(string text) { return int.TryParse(text, out _) && count > 0; }
}
