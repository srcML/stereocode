// Mutators (Method-Stereotypes wiki, Structural Mutator): set, command, and non-void-command

class Account {
    private int balance;
    private int transactions;

    // @stereotype set
    public void setBalance(int amount) { balance = amount; }

    // @stereotype command
    public void deposit(int amount) { balance += amount; transactions++; }

    // @stereotype command
    public void reset() { setBalance(0); }

    // A command may return the object itself
    // @stereotype command
    public Account withdraw(int amount) { balance -= amount; transactions++; return this; }

    // @stereotype property non-void-command
    public int withdrawAll() { int all = balance; balance = 0; transactions++; return all; }
}
