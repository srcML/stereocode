// stereocode -m writes the "// @stereotype" comments of this file, which srcml --output-src then gives back

// @stereotype data-class small-class
export class Account {
    private balance = 0;

    // @stereotype get
    get total(): number {
        return this.balance;
    }

    // @stereotype set
    @logged
    setBalance(value: number): void {
        this.balance = value;
    }

    // @stereotype property
    static twice(value: number): number {
        return 2 * value;
    }
}

// @stereotype empty
interface Named {
    // An interface of TypeScript holds no code, so its methods have no stereotypes
    name(): string;
}

// @stereotype property
export function square(value: number): number {
    return value * value;
}

namespace Shapes {
    // @stereotype literal
    export function unit(): number {
        return 1;
    }
}
