// Free functions of C and the structs and unions that -t and -u label (Free-Function-Stereotypes wiki)
#include <stdbool.h>
#include <stdio.h>

// @stereotype empty
struct point { int x; int y; };

// @stereotype empty
union value { int i; float f; };

static int counter;

// @stereotype global-command
void bump(void) { counter++; }

// @stereotype command
void set_x(struct point* p, int x) { p->x = x; }

// A pointer to a const object holds no object of the caller that the function may change
// @stereotype predicate
bool is_origin(const struct point* p) { return p->x == 0 && p->y == 0; }

// @stereotype property
static int twice(int v) { return 2 * v; }

// @stereotype literal
static int zero(void) { return 0; }

// @stereotype wrapper
void show(int v) { printf("%d\n", v); }

// @stereotype main
int main(void) { bump(); show(twice(zero())); return 0; }
