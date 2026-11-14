// Free functions (Free-Function-Stereotypes wiki): the functions of a module and the static methods of a class
// predicate, property, global-command, command, factory, literal, wrapper, empty, and unclassified

let calls = 0;

// @stereotype predicate
function isEven(value) {
    return value % 2 === 0;
}

// @stereotype property
function square(value) {
    return value * value;
}

// A name that the module declares is a global of the module
// @stereotype global-command
function addCalls(count) {
    calls += count;
}

// A write through an index of a parameter changes the array of the caller
// @stereotype command
function fill(values, value) {
    values[0] = value;
}

// @stereotype property factory
function makeWidget(style) {
    const widget = new Widget(style);
    return widget;
}

// @stereotype literal
function answer() {
    return 42;
}

// @stereotype wrapper
function report(value) {
    journal.write(value);
}

// @stereotype empty
function noop(value) {}

// console.log is an ignored call, so a function that only reads its parameter fits no rule
// @stereotype unclassified
function show(value) {
    console.log(value);
}

// A parameter that the function calls is read, so the function is no literal
// @stereotype wrapper
function run(callback) {
    callback();
}

// The value of an async function is the value that its promise gives
// @stereotype predicate
async function isReady(value) {
    return value > 0;
}

// A generator gives each value of yield
// @stereotype property
function* countdown(start) {
    yield start - 1;
}

// An exported function is a function of the module
// @stereotype property wrapper
export function quadruple(value) {
    return square(square(value));
}

// A rest parameter gathers the arguments into an array of its own, so its writes change no object of the caller
// @stereotype property
function first(...values) {
    values[0] = 0;
    return values[1];
}

// The names of a destructuring pattern are parameters
// @stereotype command
function swap([left, right], pair) {
    pair.left = right;
    left = 0;
}

// A call on an element of a parameter may change the object of the caller, as a call on the parameter itself may
// @stereotype command
function closeFirst(doors) {
    doors[0].close();
}

class Shape {
    static count = 0;
    static #created = 0;

    constructor(sides) {
        this.sides = sides;
    }

    // A static method is a free function
    // @stereotype property
    static perimeter(side, sides) {
        return side * sides;
    }

    // @stereotype factory literal
    static triangle() {
        return new Shape(3);
    }

    // @stereotype property factory
    static polygon(sides) {
        return new Shape(sides);
    }

    // A static field is a global, reached through the name of the class
    // @stereotype global-command
    static register() {
        Shape.count += 1;
    }

    // this of a static method is the class
    // @stereotype global-command
    static forget() {
        this.count = 0;
    }

    // @stereotype unclassified
    static created() {
        return Shape.#created;
    }
}
