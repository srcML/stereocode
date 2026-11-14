// Globals: the variables at file or namespace scope and the static data members, which are no state of an object
// (Free-Function-Stereotypes wiki, global-command, and Method-Stereotypes wiki, set and incidental)

int g_mode;
namespace settings { int level; }

class Registry {
    static int s_entries;
public:
    // A static data member is a global
    // @stereotype incidental
    void add() { s_entries++; }
    // A namespace that qualifies a global is no class, so the write makes no collaborator
    // @stereotype incidental
    void raise() { settings::level++; }
};
int Registry::s_entries = 0;

namespace store {
    struct Shelf {
        static int level;
        int items;
        void clearQualified();
        void clearShort();
    };
}

// A static data member of the own type that its namespace qualifies belongs to no other object
// @stereotype set
void store::Shelf::clearQualified() { store::Shelf::level = 0; items = 0; }

// @stereotype set
void store::Shelf::clearShort() { Shelf::level = 0; items = 0; }

// A parent outside the archive may declare any written name, except the name of a known global
class View : public Window {
    int width;
public:
    // @stereotype incidental
    void setMode(int mode) { g_mode = mode; }
    // @stereotype set
    void setWidth(int w) { width = w; }
    // @stereotype set
    void setHeight(int h) { height = h; }
};

// @stereotype global-command
void selectMode(int mode) { g_mode = mode; }

// @stereotype global-command
void raiseLevel(int by) { settings::level += by; }

// @stereotype global-command
void selectQualified(int mode) { ::g_mode = mode; }

// @stereotype global-command
void clearRegistry(int entries) { Registry::s_entries = entries; }

// The definition of a method of a class outside the archive changes a data member of that class, which is no global
// @stereotype literal
void Panel::clear() { m_items = 0; }

int counter = 0;

// A global hides a local of the same name whose block or statement has ended
// @stereotype global-command
void afterBlock() { { int counter = 5; counter++; } counter++; }

// @stereotype global-command
void afterLoop() { for (int counter = 0; counter < 3; ++counter) { } counter = 9; }
