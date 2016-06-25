#include"../stdafx.h"
#include"crossplatform.h"

#ifdef WIN32 

#include<winsock2.h>

//windows�ж��壺
/*
* Structure used in select() call, taken from the BSD file sys/time.h.
*/
/*
struct timeval {
long    tv_sec;
long    tv_usec;
*/
int gettimeofday(struct timeval *tp, void *tzp)
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