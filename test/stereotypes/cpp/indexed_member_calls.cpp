// A call on an element of a data member is a call on the data member, and the declaration of the data member tells the type of its elements

#include <vector>

class Slot {
    int value;
public:
    // @stereotype get
    int get() const { return value; }

    // @stereotype set
    void clear() { value = 0; }
};

class Rack {
    Slot m_slots[4];
    Slot* m_heap;
    Slot** m_table;
    Slot m_spare;
    std::vector<Slot> m_items;
    std::vector<Slot*> m_shared;
public:
    // @stereotype command collaborator
    void clearSpare() { m_spare.clear(); }

    // @stereotype command collaborator
    void clearAt(int i) { m_slots[i].clear(); }

    // @stereotype command collaborator
    void clearThrough(int i) { this->m_slots[i].clear(); }

    // @stereotype property collaborator
    int valueAt(int i) { return m_slots[i].get() + 1; }

    // The elements of a pointer have the type that the pointer points to
    // @stereotype property collaborator
    int heapValue(int i) { return m_heap[i].get() + 1; }

    // @stereotype property collaborator
    int tableValue(int i) { return m_table[i]->get() + 1; }

    // The elements of a container have the type of its last template argument
    // @stereotype property collaborator
    int itemValue(int i) { return m_items[i].get() + 1; }

    // @stereotype command collaborator
    void clearShared(int i) { m_shared[i]->clear(); }

    // A call on an element of a parameter changes no data member
    // @stereotype controller stateless
    void clearAll(Slot* others) { others[0].clear(); }
};
