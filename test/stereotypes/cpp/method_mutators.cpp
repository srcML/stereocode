// Mutators (Method-Stereotypes wiki, Structural Mutator): set, command, and non-void-command

class Account {
    int m_balance;
    int m_transactions;
public:
    // @stereotype set
    void setBalance(int balance) { m_balance = balance; }

    // @stereotype command
    void deposit(int amount) { m_balance += amount; m_transactions++; }

    // @stereotype command
    void reset() { setBalance(0); }

    // A command may return the object itself
    // @stereotype command
    Account& withdraw(int amount) { m_balance -= amount; m_transactions++; return *this; }

    // @stereotype property non-void-command
    int withdrawAll() { int all = m_balance; m_balance = 0; m_transactions++; return all; }
};
