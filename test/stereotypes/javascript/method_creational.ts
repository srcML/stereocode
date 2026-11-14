// Creational methods (Method-Stereotypes wiki): constructor and factory (JavaScript has no destructor and no copy constructor)

class Matrix {
    rows: number;

    // @stereotype constructor
    constructor(rows: number = 0) {
        this.rows = rows;
    }

    // @stereotype factory stateless
    makeEmpty(): Matrix {
        return new Matrix();
    }

    // @stereotype factory stateless
    makeSquare(count: number): Matrix {
        const result = new Matrix(count);
        return result;
    }

    // Without an annotation, the class of the created object is the return type
    // @stereotype factory stateless
    makeRow() {
        return new Matrix(1);
    }

    // new without an argument list creates an object as well
    // @stereotype factory stateless
    makeColumn() {
        return new Matrix;
    }

    // A class outside the archive makes a collaborator
    // @stereotype factory collaborator stateless
    makeView() {
        const view = new MatrixView();
        return view;
    }

    // The value of an async method is the value that its promise gives
    // @stereotype factory stateless
    async load(): Promise<Matrix> {
        return new Matrix(1);
    }

    // A return type that admits undefined names the class that the method creates
    // @stereotype factory stateless
    find(count: number): Matrix | undefined {
        if (count < 0) return undefined;
        return new Matrix(count);
    }

    // A call of a function creates no object, whatever the function returns
    // @stereotype wrapper stateless
    makeCopy(count: number) {
        return cloneMatrix(count);
    }

    // @stereotype property factory
    grown(): Matrix {
        return new Matrix(this.rows + 1);
    }
}
