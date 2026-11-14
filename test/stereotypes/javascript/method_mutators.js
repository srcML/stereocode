// Mutators (Method-Stereotypes wiki, Structural Mutator): set, command, and non-void-command

class Account {
    // @stereotype constructor
    constructor() {
        this.balance = 0;
        this.transactions = 0;
        this.history = [];
    }

    // @stereotype set
    setBalance(amount) {
        this.balance = amount;
    }

    // @stereotype command
    deposit(amount) {
        this.balance += amount;
        this.transactions++;
    }

    // @stereotype command
    reset() {
        this.setBalance(0);
    }

    // A command may return the object itself
    // @stereotype command
    withdraw(amount) {
        this.balance -= amount;
        ++this.transactions;
        return this;
    }

    // @stereotype property non-void-command
    withdrawAll() {
        const everything = this.balance;
        this.balance = 0;
        this.transactions += 1;
        return everything;
    }

    // A method that returns no value returns void
    // @stereotype command
    clear() {
        this.balance = 0;
        this.transactions = 0;
        return;
    }

    // @stereotype predicate non-void-command
    spend(amount) {
        this.balance -= amount;
        this.transactions += 1;
        return this.balance >= 0;
    }

    // A call on a data member that is no query method changes the data member
    // @stereotype command
    record(entry) {
        this.history.push(entry);
    }

    // A query method of an array changes nothing
    // @stereotype predicate
    hasRecorded(entry) {
        return this.history.includes(entry);
    }

    // delete removes a member of the data member
    // @stereotype set
    forget(index) {
        delete this.history[index];
    }

    // An operator of logical assignment writes the data member
    // @stereotype set
    ensureBalance() {
        this.balance ??= 0;
    }
}
