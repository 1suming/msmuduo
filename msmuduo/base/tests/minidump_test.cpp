#include"../../stdafx.h"
#include"../minidump.h"
#include<iostream>
using namespace std;

void test()
{
	string s = "abcd";
	try{
		s[100] = 'b';

	}
	catch (exception& e){
		cout << "with exception:[" << e.what() << "]" << endl;
	}
	catch (...)
	{
		cout << "with unkown exception" << endl;
	}

}
int main()
{
	InitMinDump(); //没有minidump产生?

	test();
	
}