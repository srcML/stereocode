// A static variable at file scope of a source file belongs to its unit, while the other globals belong to the archive
// (Free-Function-Stereotypes wiki, global-command)
static int count = 0;
int width;
int height;

// @stereotype global-command
void tick(void) { count++; }
