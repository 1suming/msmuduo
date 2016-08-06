#include"msmuduo/stdafx.h"
#include"Timer.h"


NS_USING;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
	if (repeat_)
	{
		expiration_ = addTime(now, interval_);//addTime��Timestamp�ж���Ϊȫ�ֺ���

	}
	else
	{
		expiration_ = Timestamp::invalid(); //Timestamp Ϊ0
	}
}