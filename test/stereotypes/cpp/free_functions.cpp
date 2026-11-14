// Free functions (Free-Function-Stereotypes wiki): predicate, property, global-command, command, factory,
// main, literal, wrapper, and empty

class Widget {
public:
    explicit Widget(int style);
};

void log(int value);

int g_calls;

// @stereotype predicate
bool isEven(int value) { return value % 2 == 0; }

// @stereotype property
int square(int value) { return value * value; }

// @stereotype global-command
void addCalls(int count) { g_calls += count; }

// @stereotype command
void swapValues(int& a, int& b) { int t = a; a = b; b = t; }

// @stereotype property factory
Widget* makeWidget(int style) { Widget* widget = new Widget(style); return widget; }

// @stereotype literal
int answer() { return 42; }

// @stereotype wrapper
void report(int value) { log(value); }

// @stereotype empty
void noop(int value) { }

// A parameter that the function calls is read, so the function is no literal
// @stereotype wrapper
void run(std::function<void()> callback) { callback(); }

// @stereotype main
int main(int argc, char* argv[]) { report(argc); return 0; }
