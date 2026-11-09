// Method-Stereotypes wiki, Structural Accessors, Java examples (RxJava and a JPEG decoder), each in a minimal class

class BaseTestConsumer {
    boolean timeout;

    // @stereotype get
    public final boolean isTimeout() {
        return timeout;
    }
}

final class ReplayBuffer {
    volatile int size;

    // @stereotype get
    long cachedEventCount() {
        return size;
    }
}

final class BlockingObserver {
    boolean done;
    Throwable error;

    // The wiki label is predicate; the data member error is an object of another class
    // @stereotype predicate collaborator
    public boolean hasThrowable() {
        return done && error != null;
    }
}

final class Notification<T> {
    final Object value;

    // The wiki label is predicate; the data member value is an object of another class
    // @stereotype predicate collaborator
    public boolean isOnComplete() {
        return value == null;
    }
}

interface QueueFuseable<T> {
    int SYNC = 1;
    int ASYNC = 2;
}

abstract class BasicQueueSubscription<T> implements QueueFuseable<T> {
    // The wiki label is property; ASYNC is an interface constant, which Java makes static
    // @stereotype incidental
    public final int requestFusion(int mode) {
        return mode & ASYNC;
    }
}

final class SpscArrayQueue<E> {
    final int mask;

    // @stereotype property
    int calcElementOffset(long index) {
        return (int)index & mask;
    }
}

final class JPEGDecoder {
    public static final int TQI = 2;
    public static final int[] ZigZag8x8 = {0, 1, 8, 16, 9, 2, 3, 10};
    int[][] quantizationTables;
    int[][] frameComponents;
    int[] componentIds;

    // @stereotype void-accessor
    void dequantize(int[] dataUnit, int iComp) {
    	int[] qTable = quantizationTables[frameComponents[componentIds[iComp]][TQI]];
    	for (int i = 0; i < dataUnit.length; i++) {
    		int zzIndex = ZigZag8x8[i];
    		dataUnit[zzIndex] = dataUnit[zzIndex] * qTable[i];
    	}
    }
}
