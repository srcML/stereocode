// The type of an inherited data member counts against the class of the method that uses the data member

class Element;

class Node {
protected:
    Element* m_first;
    Node* m_next;
public:
    // @stereotype predicate
    bool hasNext() const { return m_next != nullptr; }
};

class Element : public Node {
public:
    // The data member has the type of the own class
    // @stereotype get
    Element* first() const { return m_first; }

    // The data member has the type of the parent, which is another class
    // @stereotype predicate collaborator
    bool isLast() const { return m_next == nullptr; }
};
