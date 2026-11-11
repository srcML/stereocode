#include <vector>
class Stack {
    std::vector<int> items;
    int top;
public:
    // @stereotype get
    int size() const { return top; }
    // @stereotype command
    void push(int v) { items.push_back(v); top++; }
};
