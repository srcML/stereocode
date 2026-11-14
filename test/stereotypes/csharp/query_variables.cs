// The range variables of a query expression are locals

using System.Collections.Generic;
using System.Linq;

class Item {
    private int weight;

    // @stereotype get
    public int Weight() { return weight; }
}

class Catalog : System.IDisposable {
    private List<Item> items;

    // A call on a range variable is a call on another object, and let writes no data member
    // @stereotype property controller
    IEnumerable<int> Heavy() { return from item in items let weight = item.Weight() where weight > 10 select weight; }

    // @stereotype property controller
    IEnumerable<int> Typed() { return from Item item in items select item.Weight(); }

    // @stereotype property controller
    IEnumerable<int> Grouped() { return from item in items group item by item.Weight() into byWeight select byWeight.Count(); }

    // @stereotype property controller
    IEnumerable<int> Joined(List<Item> others) { return from item in items join other in others on other.Weight() equals 0 select other.Weight(); }
}
