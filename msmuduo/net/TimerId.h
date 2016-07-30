#ifndef _TimerId_h
#define _TimerId_h


#include"stdafx.h"

#include"base/copyable.h"


NS_BEGIN
class Timer;

//TimerId非常简单，它被设计用来取消Timer的，它的结构很简单，只有一个Timer指针和其序列号。


///An opaque identifier,for canceling Timer.一个不透明的标识符
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

	friend class TimerQueue; //TimerQueue为其友元，可以操作其私有数据。



private:
	Timer* timer_;
	int64_t sequence_;
};


NS_END
#endif
