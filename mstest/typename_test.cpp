#include<iostream>
#include<vector>
using namespace std;
//typename和class区别，在于从属类型
template<class T>
class Person{
	typename T::iterator iter; //为什么必须加typename，是为了反正域作用符和实际意义冲突
	typedef typename T::iterator Iter; 
 

	void output()
	{

	}
};

int main()
{
	Person<vector<int> > p;


}