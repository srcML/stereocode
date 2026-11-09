// The typedef Document of the global namespace of editor/buffer.hpp, another program of the archive, hides no class here

using namespace Scintilla;

class SearchParameters {
    Document* m_document;
public:
    // @stereotype property collaborator
    int half() { return m_document->chars() / 2; }
};
