// A source file before its header in the archive, whose definitions of static methods write the type that an alias of the header renames,
// or the alias of the class for the type that the declaration writes, and stay free functions

#include "gauge.hpp"

namespace dials {
    // @stereotype property
    double Gauge::clamp(double l) { return l * 2; }

    // @stereotype property
    long Gauge::advance(Tick t) { return t + 1; }
}
