#include<string>
#include<iostream>
#include<memory> //unique_ptr

using namespace std;
 
class A{
public:
	A(string name = "hello")
	{
		name_ = name;
	}
	~A()
	{
		cout << "A desctor" << endl;
	}

	string name_;
};


void test()
{
	//std::unique_ptr<A> p1 = new A("hello"); ×ö·¨´íÎó
	std::unique_ptr<A> p1(new A("hello"));

	auto p2 = p1.release(); //yield ownership of the pointer

	if (p1)
	{
		cout << "p1 is not empty" << endl;
	}
	cout << p2->name_ << endl;
}
int main()
{
	test();

}