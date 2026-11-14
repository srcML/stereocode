// The variable of a using declaration is a local of the type that the declaration names

using System.IO;

class Journal : System.IDisposable {
    private string path;

    // A call on a local is a call on another object
    // @stereotype controller
    void Append(string line) { using var writer = new StreamWriter(path); writer.WriteLine(line); }

    // @stereotype property controller
    string First() { using StreamReader reader = new StreamReader(path); return reader.ReadLine(); }

    // The statement form declares its variable the same way
    // @stereotype property controller
    string Second() { using (StreamReader reader = new StreamReader(path)) { reader.ReadLine(); return reader.ReadLine(); } }

    // A write to the variable changes no data member that a parent outside the archive may declare
    // @stereotype controller
    async void Flush(string line) { await using var writer = new StreamWriter(path); writer.WriteLine(line); writer = null; }
}
