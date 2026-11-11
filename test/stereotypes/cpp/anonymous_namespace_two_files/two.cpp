static int limit = 10;
namespace {
// @stereotype degenerate small-struct
struct Impl {
    // limit is the static global of this file
    // @stereotype incidental
    int max() const { return limit; }
};
}
