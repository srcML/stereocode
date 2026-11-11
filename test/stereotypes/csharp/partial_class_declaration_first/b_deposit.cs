public partial class Account {
    // OnChanged is a method of the class, declared in the other part
    // @stereotype command
    public void Deposit(int amount) {
        balance = balance + amount;
        OnChanged();
    }
}
