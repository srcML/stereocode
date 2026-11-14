// A deconstruction declares the variables or writes the variables that it names

using System.Collections.Generic;

class Range : System.IDisposable {
    private int low;
    private int high;
    private KeyValuePair<int, int> pair;
    private List<KeyValuePair<int, int>> pairs;

    // The declared variables are locals, also in a class with a parent outside the archive, which may declare any other name
    // @stereotype property collaborator
    int Width() { var (first, last) = pair; first = first + 1; return last - first; }

    // @stereotype property collaborator
    int Span() { (int first, int last) = pair; first = first + 1; return last - first; }

    // @stereotype property collaborator
    int Total() { int total = 0; foreach (var (key, weight) in pairs) { key = key * weight; total += key; } return total; }

    // @stereotype property collaborator
    int Sum() { int total = 0; foreach ((int key, int weight) in pairs) { key = key * weight; total += key; } return total; }

    // The arguments of new without a type declare no variable
    // @stereotype property factory collaborator
    KeyValuePair<int, int> Bounds(int step) { KeyValuePair<int, int> made = new(step, high); return made; }

    // A deconstruction into data members writes each of them
    // @stereotype command
    void Reset() { (low, high) = (0, 0); }

    // @stereotype command
    void Swap() { (this.low, this.high) = (high, low); }

    // @stereotype set
    void Shift(int step) { (low, _) = (low + step, step); }
}
