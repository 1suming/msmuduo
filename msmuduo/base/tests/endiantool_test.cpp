#include"../../stdafx.h"
#include"../endiantool.h"
#include<iostream>

using namespace std;


NS_USING;

int main()
{
	uint64_t a = 1;
	uint64_t a2 = endiantool::hostToNetwork64(a);

	cout << a2 << endl;
	cout << "hex:" << hex << a2 << endl;//output:100 0000 0000 0000

}