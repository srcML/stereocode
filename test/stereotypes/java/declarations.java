// Annotations, modifiers, calls of this and super in a constructor, and throws clauses leave the stereotypes of a method as its body gives them

abstract class Shape {
    protected int sides;

    // @stereotype constructor
    protected Shape(int count) { sides = count; }

    // @stereotype constructor
    protected Shape() { this(3); }

    // @stereotype get
    public synchronized int count() { return sides; }

    abstract double area();

    // @stereotype get
    @Override
    public int hashCode() { return sides; }

    // @stereotype predicate collaborator
    @Override
    public boolean equals(Object other) { return other instanceof Shape && ((Shape) other).sides == sides; }
}

final class Square extends Shape {
    // @stereotype constructor
    Square() { super(4); }

    // @stereotype property
    @Override
    final double area() { return sides * 2.5; }

    // @stereotype set
    void resize(int count) throws IllegalStateException { sides = count; }
}

// An annotation type declares its elements without bodies
@interface Marker {
    String value() default "";
}
