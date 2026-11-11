// Two classes Node in the packages a and b are two types
package a;
// @stereotype data-class small-class
public class Node {
    private int value;
    // @stereotype get
    public int getValue() { return value; }
    // @stereotype set
    public void setValue(int v) { value = v; }
}
