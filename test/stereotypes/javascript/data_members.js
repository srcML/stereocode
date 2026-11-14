// Data members: the fields of the class body, the private names that start with #, and the writes of the methods through this

class Counter {
    #count = 0;
    static #instances = 0;
    step = 1;
    onTick = () => this.#count;

    // An accessor of a property reads or writes the data members as a method does
    // @stereotype get
    get count() {
        return this.#count;
    }

    // @stereotype set
    set count(value) {
        this.#count = value;
    }

    // A private method is a method of the class
    // @stereotype command
    tick() {
        this.#count += this.step;
        this.#reset();
    }

    // @stereotype set
    #reset() {
        this.step = 1;
    }

    // A static field is no data member of the object
    // @stereotype incidental
    instances() {
        return Counter.#instances;
    }

    // A data member that holds a function is read by its call
    // @stereotype property wrapper
    tickValue() {
        return this.onTick();
    }

    // A destructuring pattern of a parameter declares the names of its keys
    // @stereotype set
    configure({ step, start = 0 }) {
        this.step = step;
    }

    // A write of the method declares a data member, also outside the constructor
    // @stereotype set
    pause() {
        this.paused = true;
    }

    // @stereotype get
    isPaused() {
        return this.paused;
    }
}

// The methods of an object literal and of a class expression are no methods of a declared class
const handlers = {
    run() {
        return 1;
    },
};

const Anonymous = class {
    run() {
        return 2;
    }
};
