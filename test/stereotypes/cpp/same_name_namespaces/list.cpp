// Two classes Node in different namespaces and files are two types
namespace list {
// @stereotype data-provider data-class small-class
class Node {
    int value;
public:
    // @stereotype get
    int get() const { return value; }
};
}
