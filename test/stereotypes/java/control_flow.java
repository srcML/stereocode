// Writes and returns inside control flow count as they do outside it, in the branches of if and switch, in loops, in try, catch,
// and finally, and in synchronized blocks
import java.util.List;

public class Counter {
    private int count;
    private int limit;
    private boolean open;

    // @stereotype set
    public void bump(boolean up) { if (up) count++; }

    // @stereotype command
    public void reset(boolean full) { if (full) count = 0; else limit = 0; }

    // @stereotype set
    public void fill(int n) { for (int i = 0; i < n; i++) count += i; }

    // @stereotype set
    public void add(List<Integer> values) { for (int value : values) count += value; }

    // @stereotype set
    public void drain() { while (count > 0) count--; }

    // @stereotype set
    public void step() { do { count++; } while (count < limit); }

    // @stereotype command
    public void apply(int code) {
        switch (code) {
            case 1: count = 0; break;
            case 2: limit = 0; break;
            default: break;
        }
    }

    // A data member returned in each branch
    // @stereotype get
    public int current(boolean upper) { if (upper) return limit; return count; }

    // @stereotype property
    public int scaled(int mode) {
        switch (mode) {
            case 0: return count * 2;
            default: return count + limit;
        }
    }

    // @stereotype predicate
    public boolean reaches(int target) {
        outer:
        for (int i = count; i < limit; i++) {
            if (i == target) return true;
            if (i < 0) break outer;
        }
        return false;
    }

    // @stereotype property
    public int bounded() { return count > limit ? limit : count; }

    // The variable of a catch clause is a local variable of its type
    // @stereotype command collaborator
    public void parse(String text) {
        try { count = Integer.parseInt(text); }
        catch (NumberFormatException error) { open = false; }
        finally { limit = 0; }
    }

    // @stereotype set
    public void close() { synchronized (this) { open = false; } }

    // A thrown object that the method creates leaves the method a set
    // @stereotype set
    public void check(int value) { if (value < 0) throw new IllegalArgumentException("negative"); count = value; }
}
