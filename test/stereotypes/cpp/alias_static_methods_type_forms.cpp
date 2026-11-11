// The definition of a static method outside its class matches its declaration also where the declaration writes a parameter type through an alias
// of a pointer, of a constant type, or of a type of several names, or through an alias in a template argument, and the definition writes the type
// that the alias stands for, so the definition stays a free function (Free-Function-Stereotypes wiki)

#include <vector>

// @stereotype data-provider data-class small-class
class Node {
    int value;
public:
    // @stereotype get
    int get() const { return value; }
};

typedef Node* NodePtr;
using ConstNodePtr = const Node*;
typedef int Count, *CountPtr;
typedef unsigned char Byte;
typedef unsigned int Natural;
using Size = unsigned long;
typedef long long Wide;
typedef char* Text;
using ConstText = const char*;

// @stereotype data-provider data-class small-class
class Tree {
    int height;
public:
    static int depth(NodePtr node);
    static int weight(ConstNodePtr node);
    static int first(CountPtr counts);
    static int low(Byte b);
    static int half(Natural n);
    static long bytes(Size n);
    static long wide(Wide w);
    static int initial(Text s);
    static int length(ConstText s);
    static int total(const std::vector<Count>& counts);
    // @stereotype get
    int getHeight() const { return height; }
};

// @stereotype property
int Tree::depth(Node* node) { return node == 0 ? 0 : 1; }
// @stereotype property
int Tree::weight(const Node* node) { return node == 0 ? 0 : 2; }
// @stereotype property
int Tree::first(int* counts) { return counts[0]; }
// @stereotype property
int Tree::low(unsigned char b) { return b + 1; }
// @stereotype property
int Tree::half(unsigned int n) { return n / 2; }
// @stereotype property
long Tree::bytes(unsigned long n) { return n * 8; }
// @stereotype property
long Tree::wide(long long w) { return w / 2; }
// @stereotype property
int Tree::initial(char* s) { return s[0]; }
// @stereotype property
int Tree::length(const char* s) { return s[0]; }
// @stereotype property wrapper
int Tree::total(const std::vector<int>& counts) { return counts.size(); }
