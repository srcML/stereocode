// A using alias directive holds only in its own file, where it hides the classes Gauge and Dial of b_panel.cs,
// while the panel and the console of b_panel.cs reach those classes

using Gauge = Shop.Parts.Meter;

namespace Shop.Parts
{
    public class Meter
    {
        // @stereotype empty
        public void Log() { }
    }
}

namespace Shop.Cars
{
    using Dial = Shop.Parts.Meter;

    public class Dashboard
    {
        private Gauge gauge;
        private Dial dial;

        // @stereotype wrapper
        public void Report() { gauge.Log(); }

        // @stereotype wrapper
        public void Check() { dial.Log(); }
    }
}
