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
