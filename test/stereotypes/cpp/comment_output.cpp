// stereocode -m writes the "// @stereotype" comments of this file, which srcml --output-src then gives back

// @stereotype data-class small-class
class Account {
    int balance;
public:
    // @stereotype get
    int getBalance() const { return balance; }
    // @stereotype set
    void setBalance(int value) { balance = value; }
    // @stereotype property
    friend int twice(const Account& account) { return 2 * account.balance; }
};

namespace bank {
	// @stereotype property
	int square(int value) { return value * value; }
}

// @stereotype data-provider data-struct small-struct
typedef struct {
    int width;
    // @stereotype get
    int getWidth() const { return width; }
} Size;
