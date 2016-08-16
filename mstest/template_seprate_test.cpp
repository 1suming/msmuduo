#include"template_seprate.h"
#include<iostream>
using namespace std;


int main()
{
	TemplateTest<int> obj;
	int a = 5;
	obj.setData(a);
	obj.output();

}