#include<iostream>
#include<vector>
#include<string>
#include<list>


#include<boost/any.hpp>

#include"AnyMy.hpp"

using namespace std;



/*
c++ģ������һ����Զ��Ƶ�������Ƶ�����Ҫ��ʾָ��.
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

	int *pb = boost::any_cast<int>(any2);//any2������ʱany*��any_cast�в�����int*,
	/*
	template<typename ValueType>
	ValueType * any_cast(any * operand) BOOST_NOEXCEPT
	���ԴӺ���ԭ���п���
	*/

	cout << *pb << endl;

 
	BOOSTANYTEST1 test1 = 5; //���Զ��Ƶ������캯��
	test1.func(6);

}

void test2()
{
	int a = 1;
	int* pa = &a;
	 
	ANYMY::Any anyTmp = a;
	ANYMY::Any* any2 = &anyTmp;

	int *pb = ANYMY::any_cast<int>(any2);//any2������ʱany*��any_cast�в�����int*,
	 
	/*
	template<typename ValueType>
	ValueType * any_cast(any * operand) BOOST_NOEXCEPT
	���ԴӺ���ԭ���п���
	*/
	cout <<endl<< *pb << endl;
 

}


int main()
{
	test1();
	test2();
}