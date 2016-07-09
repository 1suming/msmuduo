
#include<iostream>

#include<boost/function.hpp>
#include<boost/bind.hpp>

using namespace std;

typedef boost::function<void(int)> EventCallback;

EventCallback eventCallback;

/*
�������У�ԭ��δ֪
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