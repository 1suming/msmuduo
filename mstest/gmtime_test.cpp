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
 	printf("year:%d\n", 1900 + utc_tm.tm_year); //utc��1970��ʼ������tm�ṹ��¼��1900��ʼ��ʵ����Ч��ʱ����1970��ʼ��
	printf("month:%d\n", 1 + utc_tm.tm_mon);
	printf("day:%d\n", utc_tm.tm_mday);

}
int main()
{
	test();
}