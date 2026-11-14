// Calls: a method is called through this, and a call without an object calls a function

class Tank {
    constructor() {
        this.level = 0;
    }

    // @stereotype set
    fill() {
        this.level = 100;
    }

    // @stereotype get
    getLevel() {
        return this.level;
    }
}

class Engine {
    constructor() {
        this.speed = 0;
        this.parts = [];
        this.fuel = new Tank();
    }

    // @stereotype set
    setSpeed(speed) {
        this.speed = speed;
    }

    // @stereotype get
    getSpeed() {
        return this.speed;
    }

    // A call to a mutator of the class may change the object
    // @stereotype command
    stop() {
        this.setSpeed(0);
    }

    // A call to an accessor of the class reads a data member
    // @stereotype predicate
    isMoving() {
        return this.getSpeed() > 0;
    }

    // A call without an object calls a function, never a method of the class
    // @stereotype wrapper stateless
    idle() {
        stop();
    }

    // A call on a data member may change the data member
    // @stereotype command
    addPart(part) {
        this.parts.push(part);
    }

    // A query method only reads the data member that it is called on
    // @stereotype property
    spareParts() {
        return this.parts.indexOf(null);
    }

    // A call on an element of a data member is a call on the data member, which a query method only reads
    // @stereotype command
    tightenFirst() {
        this.parts[0].tighten();
    }

    // @stereotype property
    firstName() {
        return this.parts[0].trim();
    }

    // The class of a data member tells the method that a call on the data member calls
    // @stereotype command collaborator
    refuel() {
        this.fuel.fill();
    }

    // @stereotype predicate collaborator
    hasFuel() {
        return this.fuel.getLevel() > 0;
    }

    // Optional chaining reaches the same member as the plain operator
    // @stereotype command collaborator
    refuelIfPresent() {
        this.fuel?.fill();
    }

    // A call on a parameter, on a local, or on the result of a call is a call on another object
    // @stereotype controller stateless
    attach(hose) {
        hose.connect();
    }

    // @stereotype controller stateless
    drain() {
        const hose = findHose();
        hose.open();
    }

    // @stereotype controller stateless
    vent() {
        findHose().open();
    }

    // A call of a function of a module or of the environment is a call of a free function
    // @stereotype wrapper stateless
    wait() {
        timers.sleep(1);
    }

    // new creates an object, which is no call
    // @stereotype set collaborator
    replaceTank() {
        this.fuel = new Tank();
    }

    // @stereotype stateless
    fail() {
        throw new Error("no fuel");
    }

    // console.log is an ignored call
    // @stereotype unclassified
    show() {
        console.log(this.speed);
    }

    // process.exit is ignored, while exit of another object is a call
    // @stereotype unclassified
    quit() {
        console.log(this.speed);
        process.exit(1);
    }

    // @stereotype controller
    leave(door) {
        console.log(this.speed);
        door.exit();
    }
}

class Truck extends Engine {
    // A call of super in a constructor calls the constructor of the parent
    // @stereotype constructor
    constructor() {
        super();
        this.load = 0;
    }

    // A call through super calls the method of the parent
    // @stereotype command
    halt() {
        super.stop();
    }

    // @stereotype predicate
    isLoaded() {
        return super.getSpeed() > 0 && this.load > 0;
    }

    // The value of a call is no Boolean by its form, so a method without an annotation that returns it is no predicate
    // @stereotype property
    isLoadedAndMoving() {
        return this.load > 0 && super.isMoving();
    }
}

class Ratio {
    constructor(top, bottom) {
        this.top = top;
        this.bottom = bottom;
    }

    static gcd(a, b) {
        return b === 0 ? a : b;
    }

    // A call of a static method through the name of the class is a call of a function, which changes no object
    // @stereotype property wrapper
    reduced() {
        return this.top / Ratio.gcd(this.top, this.bottom);
    }
}

// A method named get or set is a method of the class, unlike an accessor of a property
class Cache {
    constructor() {
        this.entries = new Map();
    }

    // @stereotype property
    get(key) {
        return this.entries.get(key);
    }

    // @stereotype command
    set(key, value) {
        this.entries.set(key, value);
    }

    // @stereotype predicate
    has(key) {
        return this.entries.has(key);
    }

    // @stereotype command
    refresh(key) {
        this.set(key, this.get(key));
    }
}
