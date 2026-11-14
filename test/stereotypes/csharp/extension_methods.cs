// A call of an extension method of the archive on a data member, as basket.IsEmpty() for IsEmpty(this Basket basket), passes the
// data member to a static method, as BasketExtensions.IsEmpty(basket) does, unless the type of the data member has a method of the name
using System.Collections.Generic;

public class Basket
{
    public int items;

    // @stereotype predicate
    public bool IsFull() { return items > 9; }
}

public class Crate : Basket { }

public static class BasketExtensions
{
    public static bool IsEmpty(this Basket basket) { return basket.items == 0; }
    public static bool IsFull(this Basket basket) { return basket.items > 99; }
    public static int Total(this List<int> values, int start) { return start + values.Count; }
    public static void SetTarget(this System.WeakReference reference, object target) { reference.Target = target; }
}

public class Shopper
{
    private Basket basket;
    private Crate crate;
    private List<int> prices;
    private System.WeakReference<Basket> cache;

    // @stereotype predicate wrapper
    public bool Idle() { return basket.IsEmpty(); }

    // @stereotype predicate wrapper
    public bool IdleStatic() { return BasketExtensions.IsEmpty(basket); }

    // The extended type may be a parent of the type of the data member
    // @stereotype predicate wrapper
    public bool CrateIdle() { return crate.IsEmpty(); }

    // A method of the type of the data member hides an extension method of its name
    // @stereotype predicate collaborator
    public bool Full() { return basket.IsFull(); }

    // @stereotype property wrapper
    public int Sum() { return prices.Total(0); }

    // An extension method of WeakReference takes no object of WeakReference<T>, whose own method SetTarget changes the data member
    // @stereotype command collaborator
    public void Remember() { cache.SetTarget(basket); }
}
