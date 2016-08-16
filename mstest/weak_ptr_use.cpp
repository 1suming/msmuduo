#include<iostream>
#include<boost/shared_ptr.hpp>
#include<boost/weak_ptr.hpp>

using namespace std;

class A{
public:
	A()
	{
		cout << "A construct";
	}
	~A()
	{
		cout << "A destruct";
	}
};
typedef shared_ptr<A> APtr;
typedef weak_ptr<A> WeakAPtr;


int main()
{
	APtr aPtr(new A());

	cout << "a usecount:" << aPtr.use_count() << endl;

	WeakAPtr weakAPtr(aPtr);

	cout << "weak:" << weakAPtr.use_count() << endl;

}