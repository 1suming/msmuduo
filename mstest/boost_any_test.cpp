#include<iostream>
#include<vector>
#include<string>
#include<list>


#include<boost/any.hpp>

#include"AnyMy.hpp"

using namespace std;



/*
c++模板类型一般会自动推导，如果推导不到要显示指明.
*/
class BOOSTANYTEST1{
public:
	template<typename ValueType>
	BOOSTANYTEST1(const ValueType& val)
	{
		cout << "construct";
	}

	template<typename T>
	void func(T x)
	{
		cout << x << endl;
	}


};
void test1()
{
	int a = 1;
	int* pa = &a;
	/*
	boost::any any1 = pa;

	int *pb= boost::any_cast<int*>(any1);
	*/

	boost::any anyTmp = a;
	boost::any* any2 = &anyTmp;

	int *pb = boost::any_cast<int>(any2);//any2的类型时any*，any_cast中不能用int*,
	/*
	template<typename ValueType>
	ValueType * any_cast(any * operand) BOOST_NOEXCEPT
	可以从函数原型中看出
	*/

	cout << *pb << endl;

 
	BOOSTANYTEST1 test1 = 5; //会自动推导到构造函数
	test1.func(6);

}

void test2()
{
	int a = 1;
	int* pa = &a;
	 
	ANYMY::Any anyTmp = a;
	ANYMY::Any* any2 = &anyTmp;

	int *pb = ANYMY::any_cast<int>(any2);//any2的类型时any*，any_cast中不能用int*,
	 
	/*
	template<typename ValueType>
	ValueType * any_cast(any * operand) BOOST_NOEXCEPT
	可以从函数原型中看出
	*/
	cout <<endl<< *pb << endl;
 

}


int main()
{
	test1();
	test2();
}