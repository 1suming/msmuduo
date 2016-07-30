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
///����֤�ص������ᰴʱ����
///����Set�����Ը�Ч�����ɾ��
///TimerQueueʹ��TimerId��sequence_��timerָ������ʶһ����ʱ��
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
	///һ��Ҫ��֤�̰߳�ȫ��ͨ���������߳��б����� 
	TimerId addTimer(const TimerCallback& cb,
		Timestamp when,
		double interval);

	void cancel(TimerId timerId);

private:
	// FIXME: use unique_ptr<Timer> instead of raw pointers.
	/*
	pair�Ƚ�:Relational operators for pair
	Performs the appropriate comparison operation between the pair objects lhs and rhs.

	Two pair objects compare equal to each other if both their first members 
	compare equal to each other and both their second members compare also equal to each other 
	(in both cases using operator== for the comparison).
	Similarly, operators <, >, <= and >= perform a lexicographical comparison on the sequence
	formed by members first and second (in all cases using operator< reflexively for the comparisons).
	pair�Ƚϻ��ȱȽ�first��Ȼ���ٱȽ�second

	http://www.cplusplus.com/reference/utility/pair/operators/
	
	*/
	typedef std::pair<Timestamp, Timer*> Entry; //std::pair֧�ֱȽ�
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);
	//called when timerfd alarms      // ϵͳ��ʱ�����ڣ���ʾ��ʱ�������г�ʱ����Ķ�ʱ���Ѿ���ʱ��  
	void handleRead();
	//move out all expired timers 
	std::vector<Entry> getExpired(Timestamp now);
	// ���������ԵĶ�ʱ����ɾ��һ���Ի�ȡ���ļ�ʱ��  
	void reset(const std::vector<Entry>& expired, Timestamp now);

	bool insert(Timer* timer);

#ifdef WIN 
	void resetTimerWin(Timestamp expiration);
#endif


	EventLoop* loop_;
#ifdef LINUX 
	const int timerfd_; //Ҫ��channelǰ�涨��
#else 
	bool wsaStarupSuccess;
	int socketPairResult_;//���һ��Ҫ������wakeupFd_ǰ�棬���忴���캯��
	int wakeupFd_[2]; //windows����socketpair ,�����std::pair<socket,socket>������.
	int wakeUpFd1SetNonBlocking_;
	int wakeUpFd2SetNonBlocking_;

	HANDLE hTimer_;
#endif

	Channel timerfdChannel_;

	//Timer list sorted by expiration 
	TimerList timers_; //TimerList ʹ����expired time��Ϊkey���������Է���õ���ǰ��ʱ��timer(s)����



	//for cancel()
	ActiveTimerSet activeTimers_; //ActiveTimerSet ʹ��timer������Ϊ���ԣ���������ɾ����ʱ����


	// �Ƿ����ڴ���ʱ����  
	bool callingExpiredTimers_;//atomic 
	ActiveTimerSet cancelingTimers_;

 


};


NS_END
#endif