#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 

#ifdef WIN32
#   include <windows.h>
#else
#   include <sys/time.h>
#endif


#ifdef WIN32
/*
http://blog.sina.com.cn/s/blog_48526a5f0100iqyn.html
windowsÖÐ¶¨Òå£º
/*
* Structure used in select() call, taken from the BSD file sys/time.h.
*/
/*
struct timeval {
	long    tv_sec;        
	long    tv_usec;        
*/
int
gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
#endif



#if defined(_MSC_VER) && !defined(snprintf)
#define  snprintf  _snprintf
#endif
int main(int argc, char *argv[])
{
	struct timeval     tv;
	char         buf[] = "1970-01-01 00:00:00.000";

	struct tm *newtime;
	char log_name[128];
	char log_time[128];
	time_t lt;
	time(&lt);
	newtime = localtime(&lt);
	strftime(log_name, 128, "%Y%m%d", newtime);
	strftime(log_time, 128, "%Y-%m-%d %H:%M:%S", newtime);

	printf("%s\n", log_name);
	printf("%s\n", log_time);

	(void)gettimeofday(&tv, 0);
	time_t x = time_t(tv.tv_sec);

	newtime = localtime(&x);
	(void)strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S.000",
		newtime );

 	(void)printf("%s\n", buf);

	return (0);
}