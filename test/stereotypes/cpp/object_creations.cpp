// A creation with new without parentheses after the type, as new Matrix, is a constructor call, while an array creation calls no constructor

class Matrix {
    int m_rows;
public:
    // @stereotype factory stateless
    Matrix* make() const { return new Matrix; }

    // @stereotype factory stateless
    Matrix* makeBraced() const { return new Matrix{}; }

    // @stereotype stateless
    int* cell() const { return new int; }

    // @stereotype incidental
    int* cells() const { return new int[4]; }

    // @stereotype factory incidental
    Matrix* grid() const { return new Matrix[4]; }
};
