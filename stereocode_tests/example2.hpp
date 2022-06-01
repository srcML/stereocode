// example2.hpp
// testing for stereocode

#include <vector>
#include <iostream>
#include <string>

class example2
{
public:
	example2();
	~example2();
	std::vector< int > function1(){return dataMember1}

	std::list< std::string > function2(int i, string j);

	Processor function3(int, double, string);

	void function4(Processor, int);
	
	void voidAccessor(int&);

	Processor ProcessorFactory(int, double, string);

private:
	std::vector< int> dataMember1;
	string j;
	Processor p1;

};