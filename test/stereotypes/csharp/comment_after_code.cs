// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

// @stereotype data-class
public class Account
{
    private int balance; private int limit;
    public int Balance {
    // @stereotype get
    get { return balance; }
    // @stereotype set
    set { balance = value; } }
    // @stereotype get
    public int GetLimit() { return limit; }
    // @stereotype set
    public void SetLimit(int value) { limit = value; }
}
