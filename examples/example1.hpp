// Project 1 MESI cache coherence protocol
// This file contains class definition for Processor
// and important global variables.


#include <iostream>
#include <string>
#include <vector>
using std::string;

const int BLOCK_SIZE = 4;

class Processor{
public:
    string GetCacheState() const {return state;}
    int GetID() const {return pid;}
    int GetCache(int i) const {return cache[i];}
    void setState(string s){state = s;}
    double testingMultipleReturns(int j) const {if (j == 10){return 1.0;} else return 2.0;}
    int GetCache2(int i) {return cache[i];}
    int GetID2() {return pid;}
    
    std::vector<
    int     *   > AnotherTest(object& x){}

    std::map< int , char &  > TestingPrimitiveReturn(const helloworld){}

    

    Processor();
    Processor(int);
    void ModifyData(const int &, const int &, const string&);
    void ReadData(const int *);
    void ChangeCacheState(const string&);
    string TestingMultipleAttributeReturns(bool, bool);

    void TestingCommand();
    void IncrementDataMember();
    friend std::ostream& operator<<(std::ostream &, const Processor &);
    bool operator==(const Processor &);

private:
    string name;
    string state;
    int cache[BLOCK_SIZE];
    int pid;
    int anotherDataMember;
};
