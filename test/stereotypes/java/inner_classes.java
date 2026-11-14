// An inner class, a member class without static, reaches the data members of its enclosing instances,
// which count as data members of the inner class, as inherited data members do

class Counter {
    private int count;
    private static int created;

    // @stereotype get
    int count() { return count; }

    class Step {
        // @stereotype set
        void bump() { count++; }
        // @stereotype property
        int next() { return count + 1; }
        // @stereotype set
        void reset() { Counter.this.count = 0; }
        // A static data member of the enclosing class is a global
        // @stereotype incidental
        void note() { created++; }

        // An inner class of an inner class reaches the data members of both enclosing instances
        class Trace {
            // @stereotype property
            int doubled() { return count * 2; }
        }
    }

    // A static data member of the inner class hides the data member of the same name of the enclosing instance
    class Tally {
        static int count;
        // @stereotype incidental
        void tick() { count++; }
    }

    // A static nested class has no enclosing instance, while an inner class of it reaches its data members
    static class Registry {
        private int size;

        class Entry {
            // @stereotype set
            void grow() { size++; }
        }
    }
}

// An enum constant encloses an object of an inner class of the enum
enum Level {
    LOW;

    private int weight;

    class Probe {
        // @stereotype get
        int weight() { return weight; }
    }
}

// A call without an object in an inner class names a method of the nearest type that has a method of the name: the inner class, one of
// its parents, or the type of an enclosing instance, whose data members count as data members of the inner class
class Ledger {
    private int total;

    // @stereotype set
    void add(int amount) { total += amount; }

    // @stereotype get
    int total() { return total; }

    // @stereotype literal
    static int scale() { return 100; }

    class Entry {
        private int amount;

        // @stereotype set
        void addDirectly() { total += amount; }

        // @stereotype command
        void post() { add(amount); }

        // @stereotype command
        void postQualified() { Ledger.this.add(amount); }

        // @stereotype property
        int share() { return amount * 100 / total(); }

        // A static method of the enclosing class is a free function
        // @stereotype property wrapper
        int scaled() { return amount * scale(); }

        // The inner class defines a method total of its own, which hides the method of the enclosing class, whatever arguments the call passes
        class Line {
            private int cents;

            // @stereotype property
            int total(int factor) { return cents * factor; }

            // @stereotype property
            int twice() { return total(2); }

            // @stereotype property
            int outer() { return Ledger.this.total() + cents; }

            // The method add of the outermost class is the nearest one of its name
            // @stereotype command
            void post() { add(cents); }
        }
    }

    // A static nested class has no enclosing instance
    static class Report {
        // @stereotype wrapper stateless
        int shown() { return scale(); }
    }
}
