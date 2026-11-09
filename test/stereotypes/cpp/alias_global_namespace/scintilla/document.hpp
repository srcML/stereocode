// The class Document of another program than the typedef Document of editor/buffer.hpp

namespace Scintilla {
    class Document {
        int m_chars;
    public:
        // @stereotype get
        int chars() const { return m_chars; }
    };
}
