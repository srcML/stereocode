#include <string>
#include <iostream>
#include <vector>

void externalFunction() {
    std::cout << "This is an external function call." << std::endl;
}

void anotherExternalFunction(int dataMember) {
    std::cout << "Received Data Member: " << dataMember << std::endl;
}

class AnotherClass {
private:
    int dataMember;

public:
    AnotherClass(int value) : dataMember(value) {}

    void display() {
        std::cout << "AnotherClass data member: " << dataMember << std::endl;
    }
};

class MyClass {
private:
    int dataMember;
    int* pointerDataMember;
    int** pointerToPointerDataMember;

public:
    MyClass(int value, std::string strValue) : dataMember(value) {
        std::cout << "Constructor called with value: " << value << " and strValue: " << strValue << std::endl;
    }

    MyClass(const MyClass& other) : dataMember(other.dataMember) {
        std::cout << "Copy constructor called." << std::endl;
    }

    ~MyClass() {
        std::cout << "Destructor called for MyClass object." << std::endl;
    }

    void emptyMethod() {
        // This method intentionally left blank
    }

    void wrapExternalFunction() {
        externalFunction();
    }

    void displayNonDataMember() {
        anotherExternalFunction(dataMember);
    }
    
    MyClass* createObject(int value, std::string strValue) {
        MyClass* newObj = new MyClass(value, strValue);
        return newObj;
    }

    int getDataMember() {
        return dataMember;
    }

    int* getPointerDataMember() {
        return pointerDataMember;
    }

    int getValueOfPointerDataMember() {
        return *pointerDataMember;
    }

    int getValueOfPointerToPointerDataMember() {
        return **pointerToPointerDataMember;
    }

    bool isDataMemberPositive() {
        return dataMember > 0;
    }

    int doubleDataMember() {
        return dataMember * 2;
    }

    void addDataMember(int& param) {
        param += dataMember;
    }
    
    void callOnLocalObject() {
        MyClass localObj(5, "Local");
        std::cout << "Local object data member: " << localObj.getDataMember() << std::endl;
    }
    
    void callOnLocalObjectOfAnotherClass() {
        AnotherClass localObj(5);
        localObj.display();
    }
    
    void changeManyAttributes(int newValue) {
        dataMember = newValue;
    }
    
    void setDataMember(int newValue) {
        dataMember = newValue;
    }

    void doLocalComputation() {
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int num : numbers) {
            sum += num;
        }
        std::cout << "Sum of local numbers: " << sum << std::endl;
    }

    friend int getDataMemberFriend(const MyClass& obj);
    friend void incrementDataMember(MyClass& obj);
    friend void setDataMemberFriend(MyClass& obj, int dataMember);
};

int getDataMemberFriend(const MyClass& obj) {
    return obj.dataMember;
}

void setDataMemberFriend(MyClass& obj, int dataMember) {
    obj.setDataMember(dataMember);
    std::cout << "Data member set to " << dataMember << " via friend function." << std::endl;
}

void incrementDataMember(MyClass& obj) {
    obj.dataMember++;
    std::cout << "Data member incremented to: " << obj.getDataMember() << std::endl;
}

class C {
protected:
    int dataC;

public:
    int getDataC() { return dataC; }
};

class B : public C {
public:
    int getDataB() { return dataC; }

};

class A : public B {
public:
    int getDataA() { return dataC; }
};

union MyUnion {
    int intValue;
    float floatValue;

    void setIntValue(int value) {
        intValue = value;
    }

    int getIntValue() const {
        return intValue;
    }
};

class Example {
public:
    int getIntValue() const {
        return intValue;
    }

private:
    union {
        int intValue;
    };
};

static union {
    int intValue;
    float floatValue;
    char charValue;
};

class {
    int value;
    int getValue() {
        return value;
    }
} anonymousClass;

struct {
    int value;
    int getValue() {
        return value;
    }
} anonymousStruct;

class Base {
public:
    void display() {
        std::cout << "Base class display function" << std::endl;
    }
};

typedef class : public Base {
public:
    int value;
    int getValue() {
        return value;
    }
} TypedefClass;


class MyStaticClass {
public:
    static void staticMethod() {
        std::cout << "This is a static method." << std::endl;
    }
};

static void staticFunction() {
    std::cout << "This is a static free function." << std::endl;
}

int main() {

}


