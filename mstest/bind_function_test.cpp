
#include<iostream>

#include<boost/function.hpp>
#include<boost/bind.hpp>

using namespace std;

typedef boost::function<void(int)> EventCallback;

EventCallback eventCallback;

/*
不能运行，原因未知
*/
class A{
public:
	A()
	{
		 

	}
	void print(int x)
	{
		cout << x << endl;
	}

};
int main()
{
	A a;
	eventCallback = boost::bind(&A::print, &a);
	eventCallback(5);

}