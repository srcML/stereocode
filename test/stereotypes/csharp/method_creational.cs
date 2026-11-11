// Creational methods (Method-Stereotypes wiki): constructor, copy-constructor, destructor, and factory

public class Matrix
{
    private int rows;

    // @stereotype constructor
    public Matrix() { rows = 0; }

    // @stereotype constructor
    public Matrix(int count) { rows = count; }

    // @stereotype copy-constructor
    public Matrix(Matrix other) { rows = other.rows; }

    // @stereotype destructor
    ~Matrix() { }

    // @stereotype factory stateless
    public Matrix MakeEmpty() { return new Matrix(); }

    // @stereotype factory stateless
    public Matrix MakeSquare(int count) { Matrix result = new Matrix(count); return result; }
}
