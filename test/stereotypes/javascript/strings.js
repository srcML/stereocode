// Strings: srcML marks a template literal as one literal, so the names in its holes are read from its text

class Label {
    constructor() {
        this.text = "";
        this.width = 0;
    }

    // @stereotype property
    render() {
        return `[${this.text}]`;
    }

    // A string in quotes holds no names
    // @stereotype incidental
    template() {
        return "[${this.text}]";
    }

    // An escaped dollar sign opens no hole
    // @stereotype incidental
    escaped() {
        return `\${this.text}`;
    }

    // A hole may hold a template literal of its own
    // @stereotype property
    padded() {
        return `${this.text.padStart(`${this.width}`.length)}`;
    }

    // A name of a hole that is no data member uses no state
    // @stereotype incidental
    greeting(name) {
        return `Hello, ${name}!`;
    }

    // A string in a hole holds no names
    // @stereotype incidental
    quoted() {
        return `${"this.text"}`;
    }
}

// @stereotype property
function describe(count) {
    return `${count} items`;
}

// @stereotype literal
function banner() {
    return `${"=".repeat(20)}`;
}
