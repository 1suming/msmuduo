#include"msmuduo/stdafx.h"
#include"Timer.h"


NS_USING;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
	if (repeat_)
	{
		expiration_ = addTime(now, interval_);//addTime在Timestamp中定义为全局函数

	}
	else
	{
		expiration_ = Timestamp::invalid(); //Timestamp 为0
	}
}