namespace tree {
// @stereotype entity
class Node {
    Node* left;
    Node* right;
public:
    // @stereotype command
    void attach(Node* l, Node* r) { left = l; right = r; }
    // @stereotype command
    void clear() { left = nullptr; right = nullptr; }
    // @stereotype predicate
    bool leaf() const { return left == nullptr && right == nullptr; }
};
}
