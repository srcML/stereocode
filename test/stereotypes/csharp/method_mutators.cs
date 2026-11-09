// Mutators (Method-Stereotypes wiki, Structural Mutator): set, command, and non-void-command

public class Account
{
    private int balance;
    private int transactions;

    // @stereotype set
    public void SetBalance(int amount) { balance = amount; }

    // @stereotype command
    public void Deposit(int amount) { balance += amount; transactions++; }

    // @stereotype command
    public void Reset() { SetBalance(0); }

    // A command may return the object itself
    // @stereotype command
    public Account Withdraw(int amount) { balance -= amount; transactions++; return this; }

    // @stereotype property non-void-command
    public int WithdrawAll() { int all = balance; balance = 0; transactions++; return all; }

    public int Balance
    {
        // @stereotype set
        set { balance = value; }
    }
}
