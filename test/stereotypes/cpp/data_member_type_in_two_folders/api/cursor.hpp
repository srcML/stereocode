// A type name that fits a class of the same name in each of several folders, as the implementations for several platforms do,
// names every such class, so a call on a data member of that type reaches the methods of each class

namespace priv { class CursorImpl; }

class Cursor {
    priv::CursorImpl* m_impl;
public:
    // The method of one platform changes its object, so the call may change the data member
    // @stereotype predicate non-void-command collaborator
    bool load(int shape) { return m_impl->load(shape); }

    // The method of each platform reads or keeps its object, so the call reads the data member
    // @stereotype property collaborator
    int shape() const { return m_impl->shape() + 1; }
};
