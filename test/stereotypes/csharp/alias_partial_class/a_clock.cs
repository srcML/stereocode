// The first part of Clock, in a file whose using alias directive holds only for the code of this file

using Timer = Shop.Engine;

namespace Shop
{
    public class Engine
    {
        // @stereotype empty
        public void Log() { }
    }

    public partial class Clock
    {
        private Timer alarm;

        // @stereotype wrapper
        public void Ring() { alarm.Log(); }
    }
}
