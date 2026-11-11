// A write to a member of an element of a data member changes the data member, whose name starts the access chain
// (Method-Stereotypes wiki, set and command)

public class Entry
{
    public int First;
    public int Second;
}

public class Table
{
    private Entry[] rows;
    private int size;

    // @stereotype set collaborator
    public void SetSecond(int i, int v) { rows[i].Second = v; }
    // @stereotype set collaborator
    public void ClearFirst(int i) { this.rows[i].First = 0; }
    // @stereotype set collaborator
    public void BumpFirst(int i) { rows[i].First++; }
    // @stereotype command collaborator
    public void Append(int v) { rows[size].First = v; size++; }
}
