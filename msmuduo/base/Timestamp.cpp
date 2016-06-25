#include"../stdafx.h"

#include"Timestamp.h"

#include"crossplatform.h" //gettimeofday

#include <stdio.h> //sprinf

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#ifdef WIN
	#include<time.h>
#else
	#include <sys/time.h>

#endif



#include <boost/static_assert.hpp>

NS_USING;

//BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t)); windows平台为16

string Timestamp::toString() const
{
	char buf[32] = { 0 };
	int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
	int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
	snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
	return buf;
}


string Timestamp::toFormattedString(bool showMicroseconds) const
{
	char buf[32] = { 0 };
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	struct tm tm_time;
#ifdef WIN
	//gmtime_s(&tm_time, &seconds);//thread-safe gmtime
	localtime_s(&tm_time,&seconds); //localtime_r得到小时数会多8，因为china是+8区。同样建议使用localtime_r版本。
#else
	//gmtime_r(&seconds, &tm_time); //linux下gmtime_r为可重入版本
	localtime_s(&tm_time, &seconds); //localtime_r得到小时数会多8，因为china是+8区。同样建议使用localtime_r版本。

#endif
	if (showMicroseconds)
	{
		int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	}
	return buf;
}

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t seconds = tv.tv_sec;
	return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}
