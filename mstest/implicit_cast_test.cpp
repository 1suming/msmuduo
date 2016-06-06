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
	cout << implicit_cast<double>(i) << endl; //int转double，可以

	//implicit_cast<char*, int>(i);  //编译时compile 会提示error C2440: “return”: 无法从“const int”转换为“char *” 
}
int main()
{
	test();

}