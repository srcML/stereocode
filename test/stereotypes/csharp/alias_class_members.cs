// A nested class of a class hides a using alias directive of its name, as the name lookup of C# does

using Timer = Shop.Engine;

namespace Shop
{
    public class Engine
    {
        private int rpm;

        // @stereotype set
        public void Log() { rpm = 0; }
    }

    public class Clock
    {
        private class Timer
        {
            private int ticks;

            // @stereotype empty
            public void Log() { }
        }

        private Timer timer;

        // The nested class Clock.Timer hides the alias Timer of the file
        // @stereotype wrapper
        public void Report() { timer.Log(); }
    }
}
