#ifndef _TimerQueue_H
#define _TimerQueue_H

#include<set>

#include<boost/noncopyable.hpp>
#include"net/Callbacks.h"
#include"base/Timestamp.h"
#include"net/Channel.h"



NS_BEGIN

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///不保证回调函数会按时调用
///基于Set，可以高效插入和删除
///TimerQueue使用TimerId的sequence_或timer指针来标识一个定时器
///

class TimerQueue : boost::noncopyable
{
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	///Schedules the callback to be run at given time ,
	///repeats if @c interval >0.0
	///
	///Must be Thread Safe,Usually be called from other threads 
	///一定要保证线程安全，通常在其他线程中被调用 
	TimerId addTimer(const TimerCallback& cb,
		Timestamp when,
		double interval);

	void cancel(TimerId timerId);

private:
	// FIXME: use unique_ptr<Timer> instead of raw pointers.
	/*
	pair比较:Relational operators for pair
	Performs the appropriate comparison operation between the pair objects lhs and rhs.

	Two pair objects compare equal to each other if both their first members 
	compare equal to each other and both their second members compare also equal to each other 
	(in both cases using operator== for the comparison).
	Similarly, operators <, >, <= and >= perform a lexicographical comparison on the sequence
	formed by members first and second (in all cases using operator< reflexively for the comparisons).
	pair比较会先比较first，然后再比较second

	http://www.cplusplus.com/reference/utility/pair/operators/
	
	*/
	typedef std::pair<Timestamp, Timer*> Entry; //std::pair支持比较
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);
	//called when timerfd alarms      // 系统定时器到期（表示定时器队列中超时最早的定时器已经超时）  
	void handleRead();
	//move out all expired timers 
	std::vector<Entry> getExpired(Timestamp now);
	// 重置周期性的定时器，删除一次性或被取消的计时器  
	void reset(const std::vector<Entry>& expired, Timestamp now);

	bool insert(Timer* timer);

#ifdef WIN 
	void resetTimerWin(Timestamp expiration);
#endif


	EventLoop* loop_;
#ifdef LINUX 
	const int timerfd_; //要在channel前面定义
#else 
	bool wsaStarupSuccess;
	int socketPairResult_;//这个一定要定义在wakeupFd_前面，具体看构造函数
	int wakeupFd_[2]; //windows下用socketpair ,数组或std::pair<socket,socket>都可以.
	int wakeUpFd1SetNonBlocking_;
	int wakeUpFd2SetNonBlocking_;

	HANDLE hTimer_;
#endif

	Channel timerfdChannel_;

	//Timer list sorted by expiration 
	TimerList timers_; //TimerList 使用了expired time作为key，这样可以方便得到当前超时的timer(s)对象。



	//for cancel()
	ActiveTimerSet activeTimers_; //ActiveTimerSet 使用timer对象作为可以，方便索引删除定时器。


	// 是否正在处理超时任务  
	bool callingExpiredTimers_;//atomic 
	ActiveTimerSet cancelingTimers_;

 


};


NS_END
#endif