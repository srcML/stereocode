// Accessors (Method-Stereotypes wiki, Structural Accessor): get, predicate, property, and void-accessor

class Thermometer {
    private celsius: number;
    private calibrated: boolean;

    // @stereotype constructor
    constructor(celsius: number) {
        this.celsius = celsius;
        this.calibrated = false;
    }

    // @stereotype get
    getCelsius(): number {
        return this.celsius;
    }

    // @stereotype get
    isCalibrated(): boolean {
        return this.calibrated;
    }

    // @stereotype predicate
    isFreezing(): boolean {
        return this.celsius <= 0;
    }

    // @stereotype property
    getFahrenheit(): number {
        return this.celsius * 9 / 5 + 32;
    }

    // An accessor of a property is a method of its own, and its name is a data member for the other methods
    // @stereotype property
    get kelvin(): number {
        return this.celsius + 273;
    }

    // @stereotype get
    describe(): number {
        return this.kelvin;
    }

    // A write through an index of a parameter changes the array of the caller
    // @stereotype void-accessor
    readCelsius(values: number[]): void {
        values[0] = this.celsius;
    }
}

// The same accessors without annotations, where the returned values give the return type
class Barometer {
    // @stereotype constructor
    constructor(pressure) {
        this.pressure = pressure;
        this.ready = false;
    }

    // @stereotype get
    getPressure() {
        return this.pressure;
    }

    // A comparison is Boolean by its form
    // @stereotype predicate
    isLow() {
        return this.pressure < 1000;
    }

    // @stereotype predicate
    isUsable() {
        return !this.ready || this.pressure === 0;
    }

    // A name is not Boolean by its form, so the conjunction of two names is no Boolean
    // @stereotype property
    reading() {
        return this.ready && this.pressure;
    }

    // @stereotype property
    inBar() {
        return this.pressure / 1000;
    }

    // A parameter without an annotation shares its object with the caller
    // @stereotype void-accessor
    readPressure(values) {
        values[0] = this.pressure;
    }
}
