// stereocode -m writes the "// @stereotype" comments of this file, which srcml --output-src then gives back

// @stereotype data-class small-class
class Account {
    private int balance;

    // @stereotype get
    public int getBalance() { return balance; }

    // @stereotype set
    public void setBalance(int value) { balance = value; }

    // @stereotype property
    static int twice(int value) { return 2 * value; }
}
