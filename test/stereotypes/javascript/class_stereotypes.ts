// Class stereotypes (Class-Stereotypes wiki, rules for class stereotype identification)
// The method labels are listed to show the class signature; constructors do not count

// @stereotype entity boundary
class Measure {
    private value = 0;
    private changes = 0;

    // @stereotype property collaborator
    scaled(scale: Scale): number {
        return this.value * 2;
    }

    // @stereotype command collaborator
    adjust(scale: Scale): void {
        this.value *= 2;
        this.changes += 1;
    }

    // @stereotype get
    getValue(): number {
        return this.value;
    }
}

// @stereotype data-provider
class Circle {
    private radius = 0.0;

    // @stereotype get
    getRadius(): number {
        return this.radius;
    }

    // @stereotype predicate
    isUnit(): boolean {
        return this.radius === 1.0;
    }

    // @stereotype property
    area(): number {
        return 3.14159 * this.radius * this.radius;
    }

    // @stereotype set
    setRadius(radius: number): void {
        this.radius = radius;
    }
}

// @stereotype minimal-entity commander
class Counter {
    private count = 0;
    private steps = 0;

    // @stereotype set
    setCount(count: number): void {
        this.count = count;
    }

    // @stereotype command
    increment(): void {
        this.count += 1;
        this.steps += 1;
    }

    // @stereotype command
    decrement(): void {
        this.count -= 1;
        this.steps += 1;
    }

    // @stereotype get
    getCount(): number {
        return this.count;
    }
}

// @stereotype factory controller
class WidgetFactory {
    private style = 0;

    // @stereotype property factory collaborator
    plain(): Widget {
        const widget = new Widget(this.style);
        return widget;
    }

    // @stereotype property factory collaborator
    bold(): Widget {
        const widget = new Widget(this.style + 1);
        return widget;
    }

    // @stereotype property factory collaborator
    italic(): Widget {
        const widget = new Widget(this.style + 2);
        return widget;
    }
}

// @stereotype pure-controller
class Dispatcher {
    private pages = 0;

    // @stereotype controller
    sendFirst(printer: Printer): void {
        printer.printPages(this.pages);
    }

    // @stereotype controller
    sendSecond(printer: Printer): void {
        printer.printPages(this.pages + 1);
    }

    // @stereotype controller
    sendThird(printer: Printer): void {
        printer.printPages(this.pages + 2);
    }
}

// @stereotype data-class
class Pair {
    private first = 0;
    private second = 0;

    // @stereotype get
    getFirst(): number {
        return this.first;
    }

    // @stereotype get
    getSecond(): number {
        return this.second;
    }

    // @stereotype set
    setFirst(first: number): void {
        this.first = first;
    }
}

// @stereotype empty
class Tag {
    private id: number;

    // @stereotype constructor
    constructor() {
        this.id = 0;
    }
}
