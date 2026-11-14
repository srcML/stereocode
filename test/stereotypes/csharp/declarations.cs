// Modifiers, attributes, constructor initializers, and operators of a declaration leave the stereotypes of a method as its body gives them
using System;

public abstract class Shape
{
    protected int sides;

    // @stereotype constructor
    protected Shape(int count) { sides = count; }

    // @stereotype constructor
    protected Shape() : this(3) { }

    // @stereotype get
    [Obsolete("Use Sides")]
    public virtual int Count() { return sides; }

    public abstract double Area();

    // @stereotype get
    public override int GetHashCode() { return sides; }

    // @stereotype predicate
    public static bool operator ==(Shape left, Shape right) { return left.sides == right.sides; }

    // @stereotype predicate
    public static bool operator !=(Shape left, Shape right) { return left.sides != right.sides; }

    // @stereotype property
    public static implicit operator int(Shape shape) { return shape.sides; }
}

public sealed class Square : Shape
{
    // @stereotype constructor
    public Square() : base(4) { }

    // @stereotype get
    public sealed override int Count() { return sides; }

    // @stereotype property
    public override double Area() { return sides * 2.5; }

    // @stereotype property
    public unsafe int First(int[] values) { fixed (int* start = values) { return *start + sides; } }
}
