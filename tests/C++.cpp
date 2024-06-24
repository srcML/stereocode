#include <string>
#include <iostream>
#include <vector>

void externalFunction() {
    std::cout << "This is an external function call." << std::endl;
}

void anotherExternalFunction(const std::string& str) {
    std::cout << "Received string: " << str << std::endl;
}

class AnotherClass {
private:
    int anotherDataMember;

public:
    AnotherClass(int value) : anotherDataMember(value) {}

    void display() {
        std::cout << "AnotherClass data member: " << anotherDataMember << std::endl;
    }
};

class MyClass {
private:
    int dataMember;
    std::string otherDataMember;

public:
    MyClass(int value, std::string strValue) : dataMember(value), otherDataMember(strValue) {
        std::cout << "Constructor called with value: " << value << " and strValue: " << strValue << std::endl;
    }

    MyClass(const MyClass& other) : dataMember(other.dataMember), otherDataMember(other.otherDataMember) {
        std::cout << "Copy constructor called." << std::endl;
    }

    ~MyClass() {
        std::cout << "Destructor called for MyClass object." << std::endl;
    }

    void emptyMethod() {
        // This method intentionally left blank
    }

    void displayNonDataMember() {
        anotherExternalFunction(otherDataMember);
    }
    
    void wrapExternalFunction() {
        externalFunction();
    }

    MyClass* createObject(int value, std::string strValue) {
        MyClass* newObj = new MyClass(value, strValue);
        return newObj;
    }

    std::string getNonPrimitiveDataMember() {
        return otherDataMember;
    }

    int getDataMember() {
        return dataMember;
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
    
    void changeManyAttributes(int newValue, std::string newStrValue) {
        dataMember = newValue;
        otherDataMember = newStrValue;
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


int main() {

}


