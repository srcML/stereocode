// Two structs Impl in the anonymous namespaces of two files are two types
namespace {
// @stereotype data-provider data-struct small-struct
struct Impl {
    int limit;
    // @stereotype get
    int get() const { return limit; }
};
}
