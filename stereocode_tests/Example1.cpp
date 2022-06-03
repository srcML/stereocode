

#include "Example1.hpp"

Processor::Processor(){
    state = "Invalid";
    for (int i = 0; i < BLOCK_SIZE; ++i){
        cache[i] = -1;
    }
    pid = -1;
}
Processor::Processor(int id){
    state = "Invalid";
    for (int i = 0; i < BLOCK_SIZE; ++i){
        cache[i] = -1;
    }
    pid = id;
}
void Processor::ModifyData(const int & n, const int & i, const string & s){
    cache[i] = n;
    state = s;
    return;
}

void Processor::ReadData(const int * m){
    for (int i = 0; i < BLOCK_SIZE; ++i){
        cache[i] = m[i];
    }
    return;
}
void Processor::ChangeCacheState(const string & s){
    //std::cout <<"Changing processor: " << pid << "'s state to: "<< s << std::endl; 
    state = s;
    return;
}
string Processor::TestingMultipleAttributeReturns(bool NAME, bool STATE){
    if (NAME)
    {
        return name;
    }
    else if (STATE)
    {
        return state;
    }
    else {
        return "hello!";
    }
}

std::ostream& operator<<(std::ostream& os, const Processor & p){
    os << "Processor " << p.pid << " cache currently contains ... ";
    for (int i = 0; i < BLOCK_SIZE; ++i){
        os << p.cache[i] << " ";
    }
    os << "and is in the '" << p.state << "' state\n";
    return os;
}
bool Processor::operator==(const Processor & rhs){
    bool ret = true;
    for (int i = 0; i < BLOCK_SIZE; ++i){
        if (cache[i] != rhs.cache[i]){
            ret = false;
            break;
        }
    }
    return ret;
}
void Processor::TestingCommand(){
    int test2 = 2;
    int test = dataMember - ++pid;
    cache[0]--;
    --cache[2];

    test = cache[0] = 5;

    pid = cache[1] = anotherDataMember + pid = 7

    ++dataMember; ++anotherDataMember++;

    pid = 2;
    pid = 5;
    pid *=20;
    anotherDataMember = 50;
    anotherDataMember /= 2;
}

void Processor::IncrementDataMember(){
    ++anotherDataMember;
}