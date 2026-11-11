// The definitions write each parameter type as the declarations do, so they match their declarations and stay free functions,
// although a class of another namespace has the name of one alias and a header nearer the definitions has an alias of the other name

#include "gauge.hpp"

// @stereotype property
int Gauge::twice(Count c) { return c * 2; }

// @stereotype property
int Dial::turn(Level l) { return l + 1; }
