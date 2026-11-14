# Mutators (Method-Stereotypes wiki, Structural Mutator): set, command, and non-void-command

class Account:
    # @stereotype constructor
    def __init__(self):
        self.balance = 0
        self.transactions = 0

    # @stereotype set
    def set_balance(self, amount: int) -> None:
        self.balance = amount

    # @stereotype command
    def deposit(self, amount: int) -> None:
        self.balance += amount
        self.transactions += 1

    # @stereotype command
    def reset(self) -> None:
        self.set_balance(0)

    # A command may return the object itself
    # @stereotype command
    def withdraw(self, amount: int) -> "Account":
        self.balance -= amount
        self.transactions += 1
        return self

    # @stereotype property non-void-command
    def withdraw_all(self) -> int:
        everything = self.balance
        self.balance = 0
        self.transactions += 1
        return everything

    # Without an annotation, a method that returns no value returns None
    # @stereotype command
    def clear(self):
        self.balance = 0
        self.transactions = 0
        return

    # @stereotype predicate non-void-command
    def spend(self, amount):
        self.balance -= amount
        self.transactions += 1
        return self.balance >= 0
