// Method-Stereotypes wiki, Creational, Java example (RxJava) in a minimal class

class RefCountProcessorTckTest {
    // @stereotype factory wrapper stateless
    public Processor<Integer, Integer> createIdentityProcessor(int bufferSize) {
        UnicastProcessor<Integer> up = UnicastProcessor.create();
        return new RefCountProcessor<>(up);
    }
}
