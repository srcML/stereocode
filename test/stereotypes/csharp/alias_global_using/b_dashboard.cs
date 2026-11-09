// The data member of the dashboard has the type that the global using alias directive of a_usings.cs names

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
    public class Dashboard
    {
        private Gauge gauge;

        // @stereotype wrapper
        public void Report() { gauge.Log(); }
    }
}
