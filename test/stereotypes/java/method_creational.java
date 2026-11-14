// Creational methods (Method-Stereotypes wiki): constructor, copy-constructor, and factory (Java has no destructor)

class Matrix {
    private int rows;

    // @stereotype constructor
    public Matrix() { rows = 0; }

    // @stereotype constructor
    public Matrix(int count) { rows = count; }

    // @stereotype copy-constructor
    public Matrix(Matrix other) { rows = other.rows; }

    // @stereotype factory stateless
    public Matrix makeEmpty() { return new Matrix(); }

    // @stereotype factory stateless
    public Matrix makeSquare(int count) { Matrix result = new Matrix(count); return result; }

    // An element or a member of a created array is no object that the method returns as created
    // @stereotype property
    public Matrix firstOf() { Matrix[] made = new Matrix[rows]; return made[0]; }

    // @stereotype property
    public int lengthOf() { Matrix[] made = new Matrix[rows]; return made.length; }
}
