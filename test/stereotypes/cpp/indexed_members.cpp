// A write to a member of an element of a data member changes the data member, whose name starts the access chain
// (Method-Stereotypes wiki, set and command)

struct Entry { int first; int second; };

class Table {
    Entry m_rows[4];
    int m_size;
public:
    // @stereotype set collaborator
    void setSecond(int i, int v) { m_rows[i].second = v; }
    // @stereotype set collaborator
    void clearFirst(int i) { this->m_rows[i].first = 0; }
    // @stereotype set collaborator
    void bumpFirst(int i) { m_rows[i].first++; }
    // @stereotype set collaborator
    void decrementSecond(int i) { --m_rows[i].second; }
    // @stereotype command collaborator
    void append(int v) { m_rows[m_size].first = v; m_size++; }
    // A write to a member of an element of a local changes the local object and no data member
    // @stereotype controller incidental
    void scratch() { Entry rows[2]; rows[1].second = 3; }
};
