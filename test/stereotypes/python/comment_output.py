# stereocode -m writes the "# @stereotype" comments of this file, which srcml --output-src then gives back

# @stereotype data-class small-class
class Account:
    # @stereotype constructor
    def __init__(self):
        self.balance = 0

    # @stereotype get
    def get_balance(self):
        return self.balance
    # @stereotype set
    def set_balance(self, value):
        self.balance = value

    # @stereotype property
    @staticmethod
    def twice(value):
        return 2 * value

    # @stereotype data-provider data-class small-class
    class Limit:
        # @stereotype constructor
        def __init__(self):
            self.amount = 0

        # @stereotype get
        def get_amount(self):
            return self.amount


# @stereotype property
def square(value):
    return value * value


if __debug__:
    # @stereotype property
    def cube(value):
        return value * value * value
