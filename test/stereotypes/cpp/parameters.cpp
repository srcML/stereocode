// Free functions and their parameters (Free-Function-Stereotypes wiki): a parameter passed by reference
// changes only through a dereference, an index, or a member

// @stereotype property
int length(const char* s) { int n = 0; while (*s++) n++; return n; }
// @stereotype unclassified
void reseat(int* a) { a = 0; }
// @stereotype command
void deref(int* a, int n) { *a = n; }
// @stereotype command
void fill(int a[], int n) { a[0] = n; }
// @stereotype command
void fillPtr(int* a, int n) { a[0] = n; }

// @stereotype unclassified
void rebind(int* p) {
  p = 0;
}

// @stereotype command
void write(int* p) {
  *p = 0;
}

struct Widget { void reset(); int size() const; };

// A call on a parameter reads the parameter, and a call other than a query through a reference that is not const changes it
// @stereotype command
void resetWidget(Widget& w) {
  w.reset();
}

// @stereotype property wrapper
int widgetSize(const Widget& w) {
  return w.size();
}

// @stereotype property
int identity(int x) {
  return x;
}
