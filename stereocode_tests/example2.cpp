// example2.cpp
// test for stereocode

example2::example2(){
}

example2::~example2(){
}

std::list< std::string > example2::function2(int i, string j){
    std::list<string> ret = {j};

    return ret;
}

Processor example2::function3(int i, double j, string s){
    Processor pil;

    p1.doSomething(i,j);

    p1.doSomethingElse(s);

    outsideFunction(p1);

    return pil;
}

void example2::function4(Processor p2, int k){
    p2.doSomething(k, 11.5);

    k = 5;

    return;
}
void example2::voidAccessor(int& k){
    k = 5;
    if (true)
    {
        k += 100;
        /* code */
    }

}

Processor example2::ProcessorFactory(int i, double j, string k){
    Processor p = Processor(i,j,k);

    return p;
}

