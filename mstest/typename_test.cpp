#include<iostream>
#include<vector>
using namespace std;
//typename��class�������ڴ�������
template<class T>
class Person{
	typename T::iterator iter; //Ϊʲô�����typename����Ϊ�˷��������÷���ʵ�������ͻ
	typedef typename T::iterator Iter; 
 

	void output()
	{

	}
};

int main()
{
	Person<vector<int> > p;


}