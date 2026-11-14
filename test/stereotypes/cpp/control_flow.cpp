// Writes and returns inside control flow count as they do outside it, in the branches of if and switch, in loops, and in try and catch
#include <stdexcept>
#include <string>
#include <vector>

class Counter {
    int count;
    int limit;
    bool open;

public:
    // @stereotype set
    void bump(bool up) { if (up) count++; }

    // @stereotype command
    void reset(bool full) { if (full) count = 0; else limit = 0; }

    // @stereotype set
    void fill(int n) { for (int i = 0; i < n; ++i) count += i; }

    // @stereotype set
    void add(const std::vector<int>& values) { for (int value : values) count += value; }

    // @stereotype set
    void drain() { while (count > 0) --count; }

    // @stereotype set
    void step() { do { ++count; } while (count < limit); }

    // @stereotype command
    void apply(int code) {
        switch (code) {
        case 1: count = 0; break;
        case 2: limit = 0; break;
        default: break;
        }
    }

    // A data member returned in each branch
    // @stereotype get
    int current(bool upper) const { if (upper) return limit; return count; }

    // @stereotype property
    int scaled(int mode) const {
        switch (mode) {
        case 0: return count * 2;
        default: return count + limit;
        }
    }

    // @stereotype predicate
    bool reaches(int target) const {
        for (int i = count; i < limit; ++i)
            if (i == target) return true;
        return false;
    }

    // @stereotype property
    int bounded() const { return count > limit ? limit : count; }

    // The parameter of a catch clause is a local variable of its type
    // @stereotype command collaborator
    void parse(const std::string& text) {
        try { count = std::stoi(text); }
        catch (const std::invalid_argument& error) { open = false; }
    }
};
