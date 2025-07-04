#include <iostream>
#include "newanddelete.hpp"

class MyDemo
{
public:
	int A;
	int B;
	double L;
};


int main(int, char**)
{
    
	// they all invoke overload new and delete
    int * test2 = new int[5];
	INFO * pinfo = new INFO[10];
	MyDemo * pdemo = new MyDemo;
 
	//delete pdemo;
	//delete[] pinfo;
    //delete []test2;
		
	StreamMemoryLeak(std::cout);

	// save to file
	std::ofstream file("MemoryLeak.txt");
	StreamMemoryLeak(file);

	// auto clear all memory leak
	ClearAllMemoryLeaks();
	
    return 0;

}
