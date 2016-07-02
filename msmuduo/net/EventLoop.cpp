 
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

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	iterator_(0),


	threadId_(CurrentThread::tid())


{
	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;

	if (t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
			<< " exists in this thread " << threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}


}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
		<< " destructs in thread " << CurrentThread::tid();

	t_loopInThisThread = NULL;
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

		if (Logger::logLevel() <= Logger::TRACE)
		{
			printActiveChannels();
		}
		//TODO sort channel by priority
		eventHandling_ = true;
		for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); it++)
		{
			currentActiveChannel_ = *it;
			currentActiveChannel_->handleEvent(pollReturnTime_);


		}

		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();


	}


	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;



}

void EventLoop::quit()
{
	quit_ = true;

	if (!isInLoopThread())
	{
		//wakeup();
	}
}



void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << CurrentThread::tid();
}
