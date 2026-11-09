// The alias fleet::Motor of the header b_fleet.hpp, which this file may include, hides the class parts::Motor

namespace fleet {
    class Van {
        Motor m_motor;
    public:
        // @stereotype wrapper
        void report() { m_motor.log(); }
    };
}
