// Method-Stereotypes wiki, Degenerate, Java examples (RxJava), each in a minimal class

class FlowableRangeTckTest {
    // @stereotype incidental
    public long maxElementsFromPublisher() {
        return 1;
    }
}

class MaybeDetachTest {
    // The wiki label is wrapper stateless; a wrapper makes no call on another object,
    // so the calls on local objects make it a controller
    // @stereotype controller stateless
    public void successDetaches() throws Exception {
        Disposable d = Disposable.empty();
        final WeakReference<Disposable> wr = new WeakReference<>(d);

        TestObserver<Integer> to = new Maybe<Integer>() {
            @Override
            protected void subscribeActual(MaybeObserver<? super Integer> observer) {
                observer.onSubscribe(wr.get());
                observer.onSuccess(1);
                observer.onSuccess(2);
            };
        }
        .onTerminateDetach()
        .test();

        d = null;

        System.gc();
        Thread.sleep(200);

        to.assertResult(1);

        assertNull(wr.get());
    }
}

final class EmptyObserver {
    // @stereotype empty
    public void onSubscribe(Disposable d)
    {

    }
}
