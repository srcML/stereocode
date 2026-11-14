// A call of a variable runs the function that the variable holds, so the call reads the variable and stays a call of a function
// (Method-Stereotypes wiki, Calls; Free-Function-Stereotypes wiki, literal and wrapper)

class Button {
    constructor(onFire) {
        this.onFire = onFire;
        this.count = 0;
        this.onReset = () => 0;
    }

    // A call of a data member reads the data member, so the method uses the state of the object
    // @stereotype wrapper
    fire() {
        this.onFire();
    }

    // @stereotype property wrapper
    reset() {
        return this.onReset() + 1;
    }

    // @stereotype wrapper
    each(visit) {
        visit(this.count);
    }

    // @stereotype get
    handler() {
        return this.onFire;
    }

    // @stereotype wrapper
    start() {
        const held = this.handler();
        held();
    }

    // A local that an arrow function initializes holds code of the method itself, so its call is no call
    // @stereotype property
    twice() {
        const doubled = (value) => 2 * value;
        return doubled(this.count);
    }

    // A nested function is code of the method itself as well
    // @stereotype property
    thrice() {
        function tripled(value) {
            return 3 * value;
        }
        return tripled(this.count);
    }
}

// A call of a parameter reads the parameter, so the function is no literal, and changes no object of the caller
// @stereotype wrapper
function run(callback) {
    callback();
}

// @stereotype property
function compute(value) {
    const square = (number) => number * number;
    return square(value) + 1;
}

const parse = JSON.parse;

// @stereotype property wrapper
function load(text) {
    return parse(text);
}
