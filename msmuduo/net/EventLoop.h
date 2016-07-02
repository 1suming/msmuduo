#ifndef _EventLoop_h
#define _EventLoop_h

#include"base/Thread.h"
#include"base/Timestamp.h"
#include"base/lock.h"
#include"base/Logging.h"


#include <boost/noncopyable.hpp>
#include<boost/function.hpp>
#include<boost/bind.hpp>
#include<boost/scoped_ptr.hpp>
#include<boost/any.hpp>

NS_BEGIN
class Channel;
class Poller;

/*
Reactor:at most one loop per thread
This is a inteface class,so don't expose too much detail 
*/
class EventLoop : boost::noncopyable
{
public:
	typedef boost::function<void()> Functor;

	EventLoop();
		
	~EventLoop();

	//被调用必须和创建时的线程一样
	void loop();

	void quit();

	//safe to call from other threads
	void runInLoop(const Functor& cb);

	void queueInLoop(const Functor& cb);

	//internal usage
	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);

	void assertInLoopThread()
	{
		if (!isInLoopThread())
		{
			abortNotInLoopThread();
		}
	}
	bool isInLoopThread()const { return threadId_ == CurrentThread::tid(); }

	static EventLoop* getEventLoopOfCurrentThread();

private:
	void abortNotInLoopThread();
	void handleRead();//wake up
	void doPendingFunctors();

	void printActiveChannels() const;//debug

private:

	typedef std::vector<Channel*> ChannelList;

	bool looping_;//atomic
	bool quit_; //  /* atomic and shared between threads, okay on x86, I guess. */
	bool eventHandling_; //atomic
	bool callingPendingFunctors_; //atomic

	int64_t iterator_;
	const int threadId_;

	Timestamp pollReturnTime_;
	boost::scoped_ptr<Poller> poller_;
	//boost::scoped_ptr<TimerQueue> timerQueue_;

	int wakeupFd_;

	boost::scoped_ptr<Channel> wakeupChannel_;
	boost::any context_;

	ChannelList activeChannels_;
	Channel* currentActiveChannel_;

	
	mutex_t mutex_;
	std::vector<Functor> pendingFunctors_;//@Guarded By mutex_

};


NS_END
#endif