// The global g_total of totals.cpp, which this unit declares, is no data member of the parent outside the archive
extern int g_total;

class Meter : public Gauge {
    int value;
public:
    // @stereotype incidental
    void reset() { g_total = 0; }
    // @stereotype set
    void store(int v) { value = v; }
};
