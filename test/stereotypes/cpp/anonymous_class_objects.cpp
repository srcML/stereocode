// The name after the body of a type, as g_anon of class { ... } g_anon; names a variable:
// a global at namespace scope, a data member in a type, and a local in a function
// (Free-Function-Stereotypes wiki, global-command and literal, and Method-Stereotypes wiki, set and incidental)

class { public: int x; } g_anon;
namespace config { struct { int level; } g_settings; }
struct Point { int x; int y; } g_origin, *g_cursor;
union { int bits; float real; } g_punned;

// A parent outside the archive may declare any written name, except the name of a known global or a local
class Canvas : public Widget {
    struct { int r; int g; } color;
public:
    // @stereotype incidental
    void moveOrigin(int x) { g_origin.x = x; }
    // @stereotype incidental
    void moveCursor(int y) { g_cursor->y = y; }
    // @stereotype incidental
    void setAnon(int v) { g_anon.x = v; }
    // @stereotype set
    void setRed(int r) { color.r = r; }
    // @stereotype incidental
    void scratch(int v) { struct { int d; } local; local.d = v; }
};

// @stereotype global-command
void resetAnon(int v) { g_anon.x = v; }

// @stereotype global-command
void raise(int level) { config::g_settings.level = level; }

// A local of a type without a name is no global
// @stereotype literal
void tally() { struct { int count; } local; local.count = 1; }

// The members of a union with a declarator belong to the declared variable, unlike those of an anonymous union
// @stereotype literal
void Converter::reset() { bits = 0; }

// @stereotype global-command
void pun(float value) { g_punned.real = value; }
