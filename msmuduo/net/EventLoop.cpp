 
#include"../stdafx.h"

#include"EventLoop.h"


NS_USING;
 
namespace
{
	threadlocal EventLoop* t_loopInThisThread = 0;

	const int kPollTimeMs = 10000;


}


EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;// FIXME: what if someone calls quit() before loop() ?

	while (!quit_)
	{
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

		++iterator_;


	}


}