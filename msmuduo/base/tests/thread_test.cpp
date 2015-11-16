#include"../../stdafx.h"
#include"../Thread.h"

using namespace ms;
void func1()
{
	 
	printf("hello2,%d\n", CurrentThread::tid());
	
}
#ifdef WIN
static unsigned int __stdcall func2(void *)
{
	printf("\nfunc2\n");
	return 1;
}
#endif
int main()
{
	printf("main tid:%d,\n", CurrentThread::tid());

	Thread t1(func1, "my");
	
	t1.start();

	printf("\n tid:%d\n", t1.tid()); //linux下 tid_可能来不及赋值而输出0


	t1.join();

	printf("main tid:%d\n", CurrentThread::tid());

	#ifdef WIN
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, func2, NULL, 0,NULL);
	#endif
	sleep(3000);

}