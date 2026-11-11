// stereocode -m writes the "// @stereotype" comments of this file, which srcml --output-src then gives back

// @stereotype data-class
public class Account
{
    private int balance;

    // @stereotype get
    public int GetBalance() { return balance; }

    public int Balance
    {
        // @stereotype get
        get { return balance; }
        // @stereotype set
        set { balance = value; }
    }
}
