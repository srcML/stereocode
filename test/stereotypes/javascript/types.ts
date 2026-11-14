// Types of TypeScript: interfaces, abstract classes, generic classes, parameter properties, and namespaces
// (Class-Stereotypes wiki; Method-Stereotypes wiki, get and set)

// An interface of signatures has no method with stereotypes, so it is empty
// @stereotype empty
interface Shape {
    area(): number;
    readonly name: string;
}

// A parameter property declares a data member, and an abstract method has no body to label
// @stereotype data-provider
abstract class Base<T> implements Shape {
    abstract area(): number;

    // @stereotype constructor
    constructor(protected readonly width: number, public height = 0) {}

    // @stereotype get
    getWidth(): number {
        return this.width;
    }

    // @stereotype property
    get name(): string {
        return `base ${this.width}`;
    }

    // @stereotype predicate
    isSquare(): boolean {
        return this.width === this.height;
    }
}

// A generic parent names its class without the type arguments
// @stereotype data-provider small-class
class Square<T> extends Base<T> {
    // @stereotype property
    area(): number {
        return this.width * this.width;
    }

    // A data member of the parent is a data member of the child
    // @stereotype get
    getHeight(): number {
        return this.height;
    }
}

// A function of a namespace is a free function
namespace Geometry {
    // @stereotype literal
    export function unit(): number {
        return 1;
    }

    // @stereotype property factory
    export function square(width: number): Square<number> {
        return new Square(width);
    }
}

// An optional data member and a data member that may be undefined keep the class of their type
// @stereotype data-provider
class Node {
    next?: Node;
    value: number | undefined;

    // @stereotype get
    getNext(): Node | undefined {
        return this.next;
    }

    // @stereotype get
    getValue(): number | undefined {
        return this.value;
    }

    // @stereotype set
    setValue(value: number): void {
        this.value = value;
    }

    // @stereotype predicate
    hasNext(): boolean {
        return this.next !== undefined;
    }
}

// An overload signature and a declared function have no body, so they get no stereotypes, unlike the implementation of the overloads
declare function external(value: number): void;
export declare function exported(value: number): void;
export function parse(text: string): number;
export function parse(text: string, radix: number): number;

// @stereotype property wrapper
export function parse(text: string, radix?: number): number {
    return Number.parseInt(text, radix);
}

// @stereotype data-provider small-class
class Reader {
    constructor(source: string);

    // @stereotype constructor
    constructor(private source: string) {}

    read(count: number): string;

    // @stereotype property
    read(count?: number): string {
        return this.source.slice(0, count);
    }
}
