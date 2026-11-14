namespace priv {

class CursorImpl {
    int m_shape;
public:
    // @stereotype predicate set
    bool load(int shape) { m_shape = shape; return true; }

    // @stereotype get
    int shape() const { return m_shape; }
};

}
