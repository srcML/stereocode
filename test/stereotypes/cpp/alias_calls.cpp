// A call on a data member whose type an alias names counts by the stereotypes of the called method of the type that the alias stands for,
// which the alias reaches through the same reduction of smart pointers as a type written in full

#include <memory>

namespace Catch {
    struct BenchmarkInfo {
        int samples;
    };

    struct IStreamingReporter {
        virtual ~IStreamingReporter() = default;

        // @stereotype empty
        virtual void benchmarkStarting( BenchmarkInfo const& ) {}

        // A method with the name of a method of the smart pointer
        // @stereotype empty
        virtual void reset() {}
    };
    using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;

    class RunContext {
    public:
        void benchmarkStarting( BenchmarkInfo const& info );
        void resetReporter();
        void resetSlot();
    private:
        IStreamingReporterPtr m_reporter;
        IStreamingReporterPtr* m_slot;
    };

    // The call reaches only the empty method of the type that the smart pointer of the alias points to,
    // so it counts like a call of a free function
    // @stereotype wrapper
    void RunContext::benchmarkStarting( BenchmarkInfo const& info ) {
        m_reporter->benchmarkStarting( info );
    }

    // A call through . reaches the smart pointer that the alias stands for
    // @stereotype command collaborator
    void RunContext::resetReporter() {
        m_reporter.reset();
    }

    // A call through -> on a raw pointer to the alias reaches the smart pointer, as on a raw pointer to a std::unique_ptr
    // @stereotype command collaborator
    void RunContext::resetSlot() {
        m_slot->reset();
    }
}
