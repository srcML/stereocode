// Method-Stereotypes wiki, Collaborational, Java examples (RxJava), each in a minimal class

class FlattenCrossMapPerf {
    Flowable<Integer> flowFlatMapIterable1;

    // One call on a data member and no write is a command
    // @stereotype command collaborator
    public void flowFlatMapIterable1(Blackhole bh) {
        flowFlatMapIterable1.subscribe(new PerfConsumer(bh));
    }
}

class FlowableFlatMapCompletableAsyncPerf {
    public int count;

    // @stereotype controller
    void subscribe(Flowable<Integer> f, Blackhole bh) {
        PerfAsyncConsumer consumer = new PerfAsyncConsumer(bh);
        f.subscribe(consumer);
        consumer.await(count);
    }
}

final class ResourceObserver<T> implements Observer<T>, Disposable {
    final AtomicReference<Disposable> upstream = new AtomicReference<>();

    // The wiki shows this example under wrapper without a label line
    // @stereotype wrapper
    public void onSubscribe(@NonNull Disposable d) {
        DisposableHelper.setOnce(upstream, d);
    }
}
