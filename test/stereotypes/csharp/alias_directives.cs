// A call on a data member whose type a using alias directive names counts by the stereotypes of the called method
// of the type that the directive names

using Gauge = Shop.Parts.Meter;

namespace Shop.Parts
{
    public class Meter
    {
        private int reading;

        // @stereotype get
        public int Read() { return reading; }

        // @stereotype set
        public void Reset() { reading = 0; }

        // @stereotype empty
        public void Log() { }
    }
}

namespace Shop.Cars
{
    // A directive of a namespace, after the definition of the type that it names
    using Dial = Shop.Parts.Meter;

    public class Dashboard
    {
        private Gauge gauge;
        private Dial dial;

        // @stereotype property collaborator
        public int Speed() { return gauge.Read() * 2; }

        // @stereotype wrapper
        public void Report() { dial.Log(); }

        // @stereotype command collaborator
        public void Clear() { dial.Reset(); }
    }
}
