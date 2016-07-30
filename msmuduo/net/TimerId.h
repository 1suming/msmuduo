#ifndef _TimerId_h
#define _TimerId_h


#include"stdafx.h"

#include"base/copyable.h"


NS_BEGIN
class Timer;

//TimerId�ǳ��򵥣������������ȡ��Timer�ģ����Ľṹ�ܼ򵥣�ֻ��һ��Timerָ��������кš�


///An opaque identifier,for canceling Timer.һ����͸���ı�ʶ��
class TimerId : public copyable
{
public:
	TimerId():
		timer_(NULL),
		sequence_(0)
	{

	}
	TimerId(Timer* timer, int64_t seq):
		timer_(timer),
		sequence_(seq)
	{

	}

	// default copy-ctor, dtor and assignment are okay

	friend class TimerQueue; //TimerQueueΪ����Ԫ�����Բ�����˽�����ݡ�



private:
	Timer* timer_;
	int64_t sequence_;
};


NS_END
#endif
