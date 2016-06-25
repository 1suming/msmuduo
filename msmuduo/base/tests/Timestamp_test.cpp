#include"../../stdafx.h"
#include"../Timestamp.h"
#include<iostream>
#include<boost/operators.hpp>
using namespace std;
NS_USING;

class A{

};
class B :A{

	int64_t a;
};  
class C : public A, public boost::less_than_comparable<C>
{
	int64_t a;

};
int main()
{
	Timestamp timestamp = Timestamp::now();

	cout << sizeof(A) << endl << sizeof(B) << endl; //1 ,8
	cout << sizeof(C) << endl;//windowsƽ̨Ϊ16

	cout << "sizeof(copyable):"<<sizeof(copyable)<<endl;
	cout << "sizeof(timestamp)"<<sizeof(timestamp) << endl;

	cout << endl << endl;
	cout << "time seocnds:" << timestamp.secondsSinceEpoch() << endl;
	string str = timestamp.toFormattedString();
	cout << str << endl;

}