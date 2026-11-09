// The globals of other units with internal linkage lie out of reach, and a method of a type whose parent lies outside the archive
// reaches a global of another unit only when its own unit declares the global
// (Free-Function-Stereotypes wiki, global-command and literal, and Method-Stereotypes wiki, set and incidental)
#include "external_base.h"

extern int height;

class Widget : public ExternalBase {
    int other;
public:
    // The parent may declare count, since the static count of util.c belongs to util.c
    // @stereotype set
    void reset() { count = 0; }
    // The parent may declare width, which this unit does not declare
    // @stereotype set
    void resize(int w) { width = w; }
    // @stereotype incidental
    void grow() { height++; }
};

// @stereotype literal
void External::clear() { count = 0; }

// @stereotype literal
void touch() { hits = 1; }

// @stereotype global-command
void setWidth(int w) { width = w; }

// @stereotype global-command
void setLevel(int level) { g_level = level; }
