// Data members: locals of lambdas, C-style array fields, boxed types, and collection interfaces
// (Method-Stereotypes wiki, get, incidental, and collaborator)

import java.util.*;
import java.util.function.IntUnaryOperator;

class Calc {
    static int total = 0;
    private int data[] = new int[4];
    private Boolean ready;
    private IntUnaryOperator op = x -> { int total = x * 2; return total; };
    // The local total of the lambda is no data member, and the static total is no object state
    // @stereotype incidental
    public int peek() { return total; }
    // @stereotype get
    public int first() { return data[0]; }
    // @stereotype get
    public Boolean isReady() { return ready; }
}

class Calc3 {
    private int[] data = new int[4];
    private Integer count;
    private Boolean ready;
    // @stereotype get
    public int first() { return data[0]; }
    // @stereotype get
    public Integer getCount() { return count; }
    // @stereotype get
    public Boolean isReady() { return ready; }
}

class Bag {
    private List<Integer> list;
    private Collection<Integer> coll;
    // @stereotype get
    public List<Integer> getList() { return list; }
    // @stereotype get
    public Collection<Integer> getColl() { return coll; }
}
