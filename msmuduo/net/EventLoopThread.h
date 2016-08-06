#ifndef _EventLoopThread_h
#define _EventLoopThread_h


#include"base/lock.h"
#include"base/Thread.h"
#include <boost/noncopyable.hpp>


NS_BEGIN

class EventLoop;

//主要是开启一个线程，这个线程中有一个EventLoop，通过接口startLoop来获得
class EventLoopThread : boost::noncopyable
{
public:
	typedef boost::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
		const string& name = string());

	~EventLoopThread();


	EventLoop* startLoop();


private:
	void threadFunc();

	EventLoop* loop_;
	bool exiting_;
	Thread thread_;
	mutex_t mutex_;
	condition_var_t cond_;

	ThreadInitCallback callback_;

};

NS_END
#endif