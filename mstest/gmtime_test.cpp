#include <stdio.h>

#include <time.h>
#include<assert.h>

void test()
{
	time_t cur_time = time(NULL);
	
#ifdef WIN32
	
	struct tm utc_tm;
	gmtime_s(&utc_tm,&cur_time); //thread-safe gmtime
	
#else
#endif
 	printf("year:%d\n", 1900 + utc_tm.tm_year); //utc从1970开始，但是tm结构记录是1900开始，实际有效的时间是1970开始。
	printf("month:%d\n", 1 + utc_tm.tm_mon);
	printf("day:%d\n", utc_tm.tm_mday);

}
int main()
{
	test();
}