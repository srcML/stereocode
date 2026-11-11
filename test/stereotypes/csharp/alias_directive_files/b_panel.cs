// A using alias directive holds only in its own file, so the panel and the console reach the classes Gauge and Dial
// and not the aliases of a_dashboard.cs, also in the namespace Shop.Cars of the alias Dial

using Shop.Other;

namespace Shop.Other
{
    public class Gauge
    {
        private int level;

        // @stereotype set
        public void Log() { level = 0; }
    }

    public class Dial
    {
        private int angle;

        // @stereotype set
        public void Log() { angle = 0; }
    }
}

namespace Shop.Panels
{
    public class Panel
    {
        private Gauge gauge;

        // @stereotype command collaborator
        public void Report() { gauge.Log(); }
    }
}

namespace Shop.Cars
{
    public class Console
    {
        private Dial dial;

        // @stereotype command collaborator
        public void Report() { dial.Log(); }
    }
}
