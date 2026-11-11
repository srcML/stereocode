// Two identical copies of one header, as in a project that vendors a library twice: each copy gets
// the labels of a single copy
// @stereotype data-class small-class
class Point {
    int m_x;
    int m_y;
public:
    // @stereotype get
    int x() const { return m_x; }
    // @stereotype set
    void setX(int x) { m_x = x; }
};
