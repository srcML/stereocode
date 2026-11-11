// Each copy of catch.hpp holds its own reporter, and the alias of each copy reaches the reporter of its own copy,
// as the copy of scintilla/catch.hpp shows

#include <memory>

namespace Catch {
    struct IStreamingReporter {
        // @stereotype empty
        virtual void benchmarkStarting( int ) {}
    };
    using IStreamingReporterPtr = std::unique_ptr<IStreamingReporter>;

    class RunContext {
        IStreamingReporterPtr m_reporter;
    public:
        // @stereotype wrapper
        void benchmarkStarting( int samples ) { m_reporter->benchmarkStarting( samples ); }
    };
}
