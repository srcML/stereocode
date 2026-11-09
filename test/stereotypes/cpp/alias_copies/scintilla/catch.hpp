// Each copy of catch.hpp holds its own reporter, and the alias of each copy reaches the reporter of its own copy,
// as the copy of lexilla/catch.hpp shows

#include <memory>

namespace Catch {
    struct IStreamingReporter {
        int m_samples;

        // @stereotype set
        virtual void benchmarkStarting( int samples ) { m_samples = samples; }
    };
    using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;

    class RunContext {
        IStreamingReporterPtr m_reporter;
    public:
        // @stereotype command collaborator
        void benchmarkStarting( int samples ) { m_reporter->benchmarkStarting( samples ); }
    };
}
