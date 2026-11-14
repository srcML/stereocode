// Creational methods (Method-Stereotypes wiki): constructor, copy-constructor, destructor, and factory

#include <memory>

class Matrix {
    int     m_rows;
    double* m_cells;
public:
    // @stereotype constructor
    Matrix() : m_rows(0), m_cells(nullptr) {}

    // @stereotype constructor
    explicit Matrix(int rows) : m_rows(rows), m_cells(new double[rows]) {}

    // @stereotype copy-constructor
    Matrix(const Matrix& other) : m_rows(other.m_rows), m_cells(new double[other.m_rows]) {}

    // @stereotype destructor
    ~Matrix() { delete[] m_cells; }

    // @stereotype factory stateless
    Matrix* makeEmpty() const { return new Matrix(); }

    // @stereotype factory stateless
    Matrix* makeSquare(int rows) const { Matrix* result = new Matrix(rows); return result; }

    // An element or a member of a created array is no object that the method returns as created
    // @stereotype property
    Matrix firstOf() const { Matrix* made = new Matrix[m_rows]; return made[0]; }

    // @stereotype property
    int rowsOf() const { Matrix* made = new Matrix[m_rows]; return made->m_rows; }

    // A data member passed to a constructor, as this->m_rows is, reads the state of the object
    // @stereotype property factory
    Matrix* makeLike() const { return new Matrix(this->m_rows); }

    // The object passed to a call other than a constructor call reads the state of the object
    // @stereotype controller
    void attach(Registry& registry) const { registry.add(*this); }

    // The object passed to the constructor of a local reads no state of the object, as the object passed after new does
    // @stereotype wrapper stateless
    void bindLocal(int value) const { Binder binder(*this); notify(value); }

    // @stereotype wrapper stateless
    void bindPointer(int value) const { Binder binder(this); notify(value); }

    // @stereotype wrapper stateless
    void bindBraced(int value) const { Binder binder{*this}; notify(value); }
};

// std::make_unique and std::make_shared create the returned object as new does, while each stays a call of a free function
class Node {
    int m_value;
public:
    // @stereotype factory wrapper stateless
    std::unique_ptr<Node> clone() const { return std::make_unique<Node>(*this); }

    // @stereotype factory wrapper stateless
    std::shared_ptr<Node> share() const { auto copy = std::make_shared<Node>(*this); return copy; }

    // @stereotype factory wrapper stateless
    std::shared_ptr<Node> child() const { std::shared_ptr<Node> node; node = make_shared<Node>(); return node; }

    // A function of another library that creates an object makes no factory
    // @stereotype wrapper stateless
    std::shared_ptr<Node> other() const { return boost::make_shared<Node>(); }
};
