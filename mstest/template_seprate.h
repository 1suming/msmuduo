#ifndef  _template_seprate_h
#define _template_seprate_h

#include<iostream>
using namespace std;

template<typename T>
class TemplateTest
{
public:
	TemplateTest()
	{

	}
	void setData(T targ)
	{
		t = targ;
	}
	void output();
	
private:
	T t;
};
//下面的有错误
template<typename T>
TemplateTest::output()
{
	cout << t << endl;
}

#endif