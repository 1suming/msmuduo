
#include<iostream>

#include<boost/function.hpp>
#include<boost/bind.hpp>
#include<boost/shared_ptr.hpp>
using namespace std;

  
class A{
public:
	int a;
	A(int ra)
	{
		a = ra;
	}
};
class B{
public:
	int a;
	B(int x)
	{
		a = x;
	}

};
int main()
{
	 
	boost::shared_ptr<B> bPtr(new B(5));
	cout << bPtr.use_count() << endl;

	A a(bPtr->a);
	cout << bPtr.use_count();



}