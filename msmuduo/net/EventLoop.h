#ifndef _EventLoop_h
#define _EventLoop_h

#include"base/Thread.h"
#include"base/Timestamp.h"
#include"base/lock.h"
#include"base/Logging.h"
#include"net/TimerId.h"

#include"net/Callbacks.h"

#include <boost/noncopyable.hpp>
#include<boost/function.hpp>
#include<boost/bind.hpp>
#include<boost/scoped_ptr.hpp>
#include<boost/any.hpp>

NS_BEGIN

#if defined NETMODEL_USE_SELECE

#elif defined NETMODEL_USE_POLL

#else 
	#ifdef WIN
		#define NETMODEL_USE_POLL
 
	#else
		#define NETMODEL_USE_EPOLL
	#endif

#endif



class Channel;
class Poller;
class TimerQueue;

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

	//�����ñ���ʹ���ʱ���߳�һ��
	void loop();

	void quit();

	//safe to call from other threads
	//runInLoop��queueInLoop����runInLoop���жϵ��õ��Ƿ���loop�̣߳������������ã��������queueInLoop��functors����
	void runInLoop(const Functor& cb);
	void queueInLoop(const Functor& cb);

	/*---------------------timers-----------------*/
	///Run callbacks at 'time'
	///Safe to call from other threads
	TimerId runAt(const Timestamp& time, const TimerCallback& cb);


	///Run callbak after @c delay seconds 
	//safe to call from other threads
	TimerId runAfter(double delay, const TimerCallback& cb);

	///Run callback every @c interval seconds
	///Safe to call from other threads
	TimerId runEvery(double interval, const TimerCallback& cb);

	///Cancels the timer
	///Safe to call from other threads
	void cancel(TimerId timerId);

	/*-------------------end timers  --------------------------------*/





	//internal usage
	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);


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
	boost::scoped_ptr<TimerQueue> timerQueue_;

#ifdef LINUX 
	//�̼߳�ͨ��
	int wakeupFd_; //Linux����eventfd ,
#else  
	bool wsaStarupSuccess;
	int socketPairResult_;//���һ��Ҫ������wakeupFd_ǰ�棬���忴���캯��
	int wakeupFd_[2]; //windows����socketpair ,�����std::pair<socket,socket>������.
	int wakeUpFd1SetNonBlocking_;
	int wakeUpFd2SetNonBlocking_;

	
#endif
	//unlike in TimerQueue, which is an internal class,
	// we don't expose Channel to client.
	boost::scoped_ptr<Channel> wakeupChannel_;
	boost::any context_;

	ChannelList activeChannels_;
	Channel* currentActiveChannel_;

	
	mutex_t mutex_;
	std::vector<Functor> pendingFunctors_;//@Guarded By mutex_

};


NS_END
#endif