public class ExternalFunctions {
    public static void externalFunction() {
        System.out.println("This is an external function call.");
    }

    public static void anotherExternalFunction(String str) {
        System.out.println("Received string: " + str);
    }
}

public class AnotherClass {
    private int anotherDataMember;

    public AnotherClass(int value) {
        this.anotherDataMember = value;
    }

    public void display() {
        System.out.println("AnotherClass data member: " + this.anotherDataMember);
    }
}

public class MyClass {
    private int dataMember;
    private String otherDataMember;

    public MyClass(int value, String strValue) {
        this.dataMember = value;
        this.otherDataMember = strValue;
        System.out.println("Constructor called with value: " + value + " and strValue: " + strValue);
    }

    public MyClass(MyClass other) {
        this.dataMember = other.dataMember;
        this.otherDataMember = other.otherDataMember;
        System.out.println("Copy constructor called.");
    }

    public void emptyMethod() {
        // This method intentionally left blank
    }

    public void displayNonDataMember() {
        ExternalFunctions.anotherExternalFunction(this.otherDataMember);
    }

    public void wrapExternalFunction() {
        ExternalFunctions.externalFunction();
    }

    public static MyClass createObject(int value, String strValue) {
        return new MyClass(value, strValue);
    }

    public String getNonPrimitiveDataMember() {
        return this.otherDataMember;
    }

    public int getDataMember() {
        return this.dataMember;
    }

    public boolean isDataMemberPositive() {
        return this.dataMember > 0;
    }

    public int doubleDataMember() {
        return this.dataMember * 2;
    }

    public void addDataMember(int param) {
        param += this.dataMember;
    }

    public void callOnLocalObject() {
        MyClass localObj = new MyClass(5, "test");
        localObj.getDataMember();
    }

    public void callOnLocalObjectOfAnotherClass() {
        AnotherClass localObj = new AnotherClass(5);
        localObj.display();
    }

    public void changeManyAttributes(int newValue, String newStrValue) {
        this.dataMember = newValue;
        this.otherDataMember = newStrValue;
    }

    public void changeAttribute(int newValue) {
        this.dataMember = newValue;
    }

    public void doLocalComputation() {
        int[] numbers = {1, 2, 3, 4, 5};
        int sum = 0;
        for (int num : numbers) {
            sum += num;
        }
        System.out.println("Sum of local numbers: " + sum);
    }

    public int getDataMember() {
        return this.dataMember;
    }

    public void setDataMember(int dataMember) {
        this.dataMember = dataMember;
    }

    public String getOtherDataMember() {
        return this.otherDataMember;
    }

    public void setOtherDataMember(String otherDataMember) {
        this.otherDataMember = otherDataMember;
    }
}

public class Program {
    public static void main(String[] args) {
        
    }
}
