/*
下面的程序DEbug会出现stack overflow.为什么？

在进入readFile函数

*/


#include<iostream>
#include<stdio.h>
#include<string>
using namespace std;



const char* g_file = NULL;
 
string readaFile(const char* filename)
{

	string s1 = "hello";
	return s1; //我这里已经提早返回，为什么还是stackvoerflow  ，因为编译器插入checkstack来检查。
	FILE* fp = ::fopen(filename, "rb");
	string content;
	if (fp)
	{
		printf("a\n");
		// inefficient!!!
		const int kBufSize = 1024 * 1024; 
		char iobuf[kBufSize]; //这里的分配内存当进入readaFile函数时就已经完成分配。
		//::setbuffer(fp, iobuf, sizeof iobuf);

		char buf[kBufSize];
		size_t nread = 0;
		while (!feof(fp))
		{
			printf("b\n");
			fgets(buf, kBufSize, fp);
			content.append(buf, strlen(buf));
		}
		::fclose(fp);
	}
	return content;
	/*

	*/
}
void func()
{

	char stack_overflow[1024 * 1024 * 5];
	stack_overflow[0] = 1;
}




int main(int argc, char * argv[])
{
	if (0)
	{


		g_file = "F:/vsproject/msmuduo/Debug/a.txt";// argv[1];
		string fileContent = readaFile(g_file);
		printf("%s\n", fileContent.c_str());
	}
	else
	{
		func();
	}



}