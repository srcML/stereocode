// Writes and returns inside control flow count as they do outside it, in the branches of if and switch, in loops, and in try, catch,
// and finally
using System;
using System.Collections.Generic;

public class Counter
{
    private int count;
    private int limit;
    private bool open;

    // @stereotype set
    public void Bump(bool up) { if (up) count++; }

    // @stereotype command
    public void Reset(bool full) { if (full) count = 0; else limit = 0; }

    // @stereotype set
    public void Fill(int n) { for (int i = 0; i < n; i++) count += i; }

    // @stereotype set
    public void Add(List<int> values) { foreach (int value in values) count += value; }

    // @stereotype set
    public void Drain() { while (count > 0) count--; }

    // @stereotype set
    public void Step() { do { count++; } while (count < limit); }

    // @stereotype command
    public void Apply(int code)
    {
        switch (code)
        {
            case 1: count = 0; break;
            case 2: limit = 0; break;
        }
    }

    // A data member returned in each branch
    // @stereotype get
    public int Current(bool upper) { if (upper) return limit; return count; }

    // @stereotype property
    public int Scaled(int mode)
    {
        switch (mode)
        {
            case 0: return count * 2;
            default: return count + limit;
        }
    }

    // @stereotype predicate
    public bool Reaches(int target)
    {
        for (int i = count; i < limit; i++)
            if (i == target) return true;
        return false;
    }

    // @stereotype property
    public int Bounded() { return count > limit ? limit : count; }

    // The variable of a catch clause is a local variable of its type
    // @stereotype command collaborator
    public void Parse(string text)
    {
        try { count = int.Parse(text); }
        catch (FormatException error) { open = false; }
        finally { limit = 0; }
    }
}
