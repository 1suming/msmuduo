#include<string>
#include<iostream>

using namespace std;

template<typename To,typename From>
inline To implicit_cast(From const &f)
{
	return f;

}
void test()
{
	int i = 5;
	cout << implicit_cast<double>(i) << endl; //intתdouble������

	//implicit_cast<char*, int>(i);  //����ʱcompile ����ʾerror C2440: ��return��: �޷��ӡ�const int��ת��Ϊ��char *�� 
}
int main()
{
	test();

}