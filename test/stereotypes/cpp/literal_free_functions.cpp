// A literal uses no parameter, no global, and no static (Free-Function-Stereotypes wiki, literal, global-command, and wrapper)

void log(int value);

int g_count;

struct Registry {
    static int s_entries;
};

// @stereotype literal wrapper
void greet() { log(42); }

// @stereotype wrapper
void report() { log(g_count); }

// @stereotype wrapper
void reportEntries() { log(Registry::s_entries); }

// A static local keeps its value between calls, as a global does
// @stereotype global-command
int next() { static int calls = 0; return ++calls; }

// @stereotype global-command
void resetCount() { g_count = 0; }
