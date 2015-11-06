#include"../../stdafx.h"
#include"../Thread.h"

using namespace ms;
void func1()
{
	printf("hello,%d", GetCurrentThreadId()); //可以运行
	printf("hello2,%d\n", CurrentThread::tid());
	
}
static unsigned int __stdcall func2(void *)
{
	printf("\nfunc2\n");
	return 1;
}
int main()
{
	printf("main tid:%d,thread id:%d\n", CurrentThread::tid(), GetCurrentThreadId());

	Thread t1(func1, "my");
	
	t1.start();

	printf("\n tid:%d\n", t1.tid());


	t1.join();

	printf("main tid:%d,thread id:%d\n", CurrentThread::tid(), GetCurrentThreadId());

	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, func2, NULL, 0,NULL);

	sleep(3000);

}