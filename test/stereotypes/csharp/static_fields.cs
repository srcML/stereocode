// Static fields and constants are globals, which are no state of an object, also in a type whose parent lies outside the archive
// (Free-Function-Stereotypes wiki, global-command, and Method-Stereotypes wiki, set and incidental)
using static Shop.Settings;

namespace Shop
{
    static class Settings
    {
        public static int Level;
        public const int Max = 3;
    }

    class Basket : Collection
    {
        int count;

        // @stereotype incidental
        public void Raise() { Level = Max; }

        // @stereotype set
        public void Add(int n) { count = n; }

        // @stereotype set
        public void Mark() { marked = true; }
    }

    class Stats
    {
        static int hits;

        // @stereotype global-command
        public static void Hit(int n) { hits += n; }

        // @stereotype global-command
        public static void Configure(int level) { Settings.Level = level; }

        // @stereotype global-command
        public static void ConfigureQualified(int level) { Shop.Settings.Level = level; }
    }

    class Limits
    {
        public static int Max;
        int used;

        // A static field of the own type that its namespace qualifies belongs to no other object
        // @stereotype set
        public void ResetQualified() { Shop.Limits.Max = 0; used = 1; }

        // @stereotype set
        public void ResetShort() { Limits.Max = 0; used = 1; }
    }

    class Verbs
    {
        public const string Skip = "Skip";
    }

    // A name without a qualifier reaches the static fields of the types around it, of their ancestors, and of static imports
    static class Checks
    {
        // Skip names a class of a library, since the constant of Verbs lies out of reach
        // @stereotype literal wrapper
        public static void Check() { Skip.IfNot(true); }
    }
}
