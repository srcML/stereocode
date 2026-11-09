// Method-Stereotypes wiki, Structural Mutators, Java examples (RxJava), each in a minimal class

final class DisposableObserver {
    boolean done;

    // @stereotype set
    public void setDone()
    {
        this.done = true;
    }
}

final class CountingConsumer {
    int items;

    // @stereotype set
    public void accept(Integer t)
    {
        items++;
    }
}

class InputWithIncrementingInteger {
    Flowable<Integer> flowable;
    Observable<Integer> observable;
    Single<Integer> single;
    Maybe<Integer> maybe;
    Completable completable;

    // The wiki label is command; the data members are objects of other classes
    // @stereotype command collaborator
    public void setup() {
        flowable = Flowable.just(1);
        observable = Observable.just(1);
        single = Single.just(1);
        maybe = Maybe.just(1);
        completable = Completable.complete();
    }
}
