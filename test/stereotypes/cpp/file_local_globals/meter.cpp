// A variable of an unnamed namespace belongs to its unit (Free-Function-Stereotypes wiki, global-command)
namespace { int hits; }
namespace stats { namespace { int misses; } }

// @stereotype global-command
void record() { hits++; }

// @stereotype global-command
void miss() { stats::misses++; }
