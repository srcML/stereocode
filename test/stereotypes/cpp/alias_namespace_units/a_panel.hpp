// The alias app::Widget of b_render.cpp, a source file that no other file includes, hides no class of this file

class Widget {
    int m_size;
public:
    // @stereotype set
    void log() { m_size = 0; }
};

namespace app {
    class Panel {
        Widget m_widget;
    public:
        // @stereotype command collaborator
        void report() { m_widget.log(); }
    };
}
