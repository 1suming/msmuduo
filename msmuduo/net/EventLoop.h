#ifndef _EventLoop_h
#define _EventLoop_h

#include <boost/noncopyable.hpp>
#include<boost/function.hpp>
#include<boost/bind.hpp>

 
namespace ms
{ 
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

private:

};


}//end namespace

#endif