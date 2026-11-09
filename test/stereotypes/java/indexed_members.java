// A write to a member of an element of a data member changes the data member, whose name starts the access chain
// (Method-Stereotypes wiki, set and command)

class Entry { int first; int second; }

class Table {
    private Entry[] rows;
    private int size;

    // @stereotype set collaborator
    void setSecond(int i, int v) { rows[i].second = v; }
    // @stereotype set collaborator
    void clearFirst(int i) { this.rows[i].first = 0; }
    // @stereotype set collaborator
    void bumpFirst(int i) { rows[i].first++; }
    // @stereotype command collaborator
    void append(int v) { rows[size].first = v; size++; }
    // A write to a member of an element of a parameter changes the object of the caller and no data member
    // @stereotype controller incidental
    void fill(Entry[] others) { others[0].first = 1; }
}
