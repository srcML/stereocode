using System;

public static class ExternalFunctions {
    public static void ExternalFunction() {
        Console.WriteLine("This is an external function call.");
    }

    public static void AnotherExternalFunction(string str) {
        Console.WriteLine("Received string: " + str);
    }
}

public class AnotherClass {
    private int anotherDataMember;

    public AnotherClass(int value) {
        anotherDataMember = value;
    }

    public void Display() {
        Console.WriteLine("AnotherClass data member: " + anotherDataMember);
    }
}

public class MyClass {
    private int dataMember;
    private string otherDataMember;

    public MyClass(int value, string strValue) {
        dataMember = value;
        otherDataMember = strValue;
        Console.WriteLine("Constructor called with value: " + value + " and strValue: " + strValue);
    }

    public MyClass(MyClass other) {
        dataMember = other.dataMember;
        otherDataMember = other.otherDataMember;
        Console.WriteLine("Copy constructor called.");
    }

    ~MyClass() {
        Console.WriteLine("Destructor called for MyClass object.");
    }

    public void EmptyMethod() {
        // This method intentionally left blank
    }

    public void DisplayNonDataMember() {
        ExternalFunctions.AnotherExternalFunction(otherDataMember);
    }

    public void WrapExternalFunction() {
        ExternalFunctions.ExternalFunction();
    }

    public MyClass CreateObject(int value, string strValue) {
        return new MyClass(value, strValue);
    }

    public string GetNonPrimitiveDataMember() {
        return otherDataMember;
    }

    public int GetDataMember() {
        return dataMember;
    }

    public bool IsDataMemberPositive() {
        return dataMember > 0;
    }

    public int DoubleDataMember() {
        return dataMember * 2;
    }

    public void AddDataMember(ref int param) {
        param += dataMember;
    }

    public void CallOnLocalObject() {
        var localObj = new MyClass(5, "test");
        localObj.GetDataMember();
    }

    public void CallOnLocalObjectOfAnotherClass() {
        var localObj = new AnotherClass(5);
        localObj.Display();
    }

    public void ChangeManyAttributes(int newValue, string newStrValue) {
        dataMember = newValue;
        otherDataMember = newStrValue;
    }

    public void ChangeAttribute(int newValue) {
        dataMember = newValue;
    }

    public void DoLocalComputation() {
        var numbers = new int[] {1, 2, 3, 4, 5};
        int sum = 0;
        foreach (var num in numbers) {
            sum += num;
        }
        Console.WriteLine("Sum of local numbers: " + sum);
    }

    public int DataMember {
        get { return dataMember; }
        set { dataMember = value; }
    }
 
    public string OtherDataMember {
        get { return this.otherDataMember; }
        set { otherDataMember = value; }
    }
}

public class Program {
    public static void Main() {

    }
}
