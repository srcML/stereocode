// A typedef of the global namespace names the document of the editor in its own file, while the search of scintilla/search.cpp,
// another program of the archive, keeps the class Document that its using directive reaches

class EditorDocument {
    int m_handle;
public:
    // @stereotype empty
    void release() { }
};

typedef EditorDocument* Document;

class Buffer {
    Document m_document;
public:
    // @stereotype wrapper
    void close() { m_document->release(); }
};
