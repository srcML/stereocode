// The second part of Clock, whose data member has the class Shop.Devices.Timer that the using directive of this file brings in

using Shop.Devices;

namespace Shop.Devices
{
    public class Timer
    {
        private int ticks;

        // @stereotype set
        public void Log() { ticks = 0; }
    }
}

namespace Shop
{
    public partial class Clock
    {
        private Timer timer;

        // @stereotype command collaborator
        public void Report() { timer.Log(); }
    }
}
