#include"stdafx.h"
#include"TimerQueue.h"


#include"base/Logging.h"
#include"net/Timer.h"
#include"net/TimerId.h"
#include"net/EventLoop.h"


#include<time.h>

#include<boost/bind.hpp>

#include"base/sockettool.h"

 

#ifdef LINUX
#include<sys/timerfd.h>

#endif

NS_BEGIN
namespace detail
{
#ifdef LINUX
	int createTimerfd()
	{
	
		int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
			TFD_NONBLOCK | TFD_CLOEXEC);
		if (timerfd < 0)
		{
			LOG_SYSFATAL << "Failed in timerfd_create";
		}
		return timerfd;
	
	}

#endif

	struct timespec howMuchTimeFromNow(Timestamp when)
	{
		int64_t microseconds = when.microSecondsSinceEpoch()
			- Timestamp::now().microSecondsSinceEpoch();
		if (microseconds < 100)
		{
			microseconds = 100;
		}
 
		//把相差时间转成timespec结构
		struct timespec ts; //windows下面为自己定义的
		ts.tv_sec = static_cast<time_t>(
			microseconds / Timestamp::kMicroSecondsPerSecond);
		ts.tv_nsec = static_cast<long>(
			(microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
		return ts;

	}
#ifdef LINUX


	void resetTimerfd(int timerfd, Timestamp expiration)
	{
		//wake up loop by timerfd_settime
		struct itimerspec newValue;
		struct itimerspec oldValue;

		bzero(&newValue, sizeof newValue);
		bzero(&oldValue, sizeof oldValue);
		newValue.it_value = howMuchTimeFromNow(expiration);
		int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
		if (ret)
		{
			LOG_ERR << "timerfd_settime()";
		}
	}
	

#endif


#ifdef WIN 

#endif
	//windows和linux可以公用
	void readTimerfd(int timerfd, Timestamp now)
	{
		uint64_t howmany;
		ssize_t n = sockettool::read(timerfd, &howmany, sizeof howmany);
		LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();

		if (n != sizeof howmany)
		{

			LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
		}
	}

}


NS_END

NS_USING;

TimerQueue::TimerQueue(EventLoop* loop):
	loop_(loop),
	#ifdef LINUX
	     timerfd_(createTimerfd()),
	     timerfdChannel_(loop,timerfd_),
	#else 
		wsaStarupSuccess(executeWSAStartup()),
		socketPairResult_(socketpair(AF_INET, SOCK_STREAM, 0, wakeupFd_)),//执行这个会把wakeupFd_赋值 ,windows下面类似EventLoop的Wakeup Channel
		wakeUpFd1SetNonBlocking_(make_socket_nonblocking(wakeupFd_[0])),
		wakeUpFd2SetNonBlocking_(make_socket_nonblocking(wakeupFd_[0])),

		timerfdChannel_(loop, wakeupFd_[0]), //把wakeupFd_[0]新建一个Channel
		hTimer_(NULL),//windows handle

	#endif
	timers_(),
	callingExpiredTimers_(false)
{
#ifdef WIN 
	if (socketPairResult_ != 0)
	{
		LOG_FATAL << "EventLoop constructor socketPairResult error";
	}
	assert(wakeUpFd1SetNonBlocking_ == 0 && wakeUpFd2SetNonBlocking_ == 0);

#endif

	timerfdChannel_.setReadCallback(
		boost::bind(&TimerQueue::handleRead, this));
	//// we are always reading the timerfd, we disarm it with timerfd_settime.
	timerfdChannel_.enableReading();




}

TimerQueue::~TimerQueue()
{
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();


#ifdef LINUX
	::close(timerfd_);
#else 
	sockettool::close(wakeupFd_[0]);
	sockettool::close(wakeupFd_[1]);
#endif

	// do not remove channel, since we're in EventLoop::dtor();
	for (TimerList::iterator it = timers_.begin(); it != timers_.end(); it++)
	{
		delete it->second;
	}
}


TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval)
{
	Timer* timer = new Timer(cb, when, interval);

	loop_->runInLoop(
		boost::bind(&TimerQueue::addTimerInLoop, this, timer));


	return TimerId(timer, timer->sequence());

}

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(
		boost::bind(&TimerQueue::cancelInLoop, this, timerId));

}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();

	bool earliestChanged = insert(timer);

	if (earliestChanged) //如果插入的timer是最早的，则改变定时器到期时间
	{
#ifdef LINUX
		resetTimerfd(timerfd_, timer->expiration());
#else 
		resetTimerWin(timer->expiration());
#endif

	}
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	ActiveTimer timer(timerId.timer_, timerId.sequence_);
	ActiveTimerSet::iterator it = activeTimers_.find(timer);

	if (it != activeTimers_.end()) //要取消的在当前激活的定时器中
	{
		size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
		assert(n == 1); (void)n;
		delete it->first;//FIXME: no delete please 
		activeTimers_.erase(it);

	}
	else if (callingExpiredTimers_)  
	{
		//  如果没有这个定时器（表示它已经超时，正在被处理），而且又是正在处理已超时定时器，那么把它插入到正在取消的定时器对列中  
		// 等待处理超时定时器的工作完成，调用reset的时候会将其删除  
		cancelingTimers_.insert(timer);
		/*
		删除定时器逻辑：

		1. 如果在ActiveTimerSet 中，表示当前定时器还未超时，直接从上面两个set中予以删除。
		2. 如果是在定时器回调函数中删除定时器，那么先保存到cancelingTimers_ set中，等待本次超时所有timers回调函数执行完毕，这时候删除的只能是循环定时器，不会有下次超时了。

		
		*/
	}

	assert(timers_.size() == activeTimers_.size());


}

/*---------------------------------------------*/

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());


#ifdef LINUX 
	detail::readTimerfd(timerfd_,now);
#else 
	detail::readTimerfd(wakeupFd_[0], now);

#endif

	std::vector<Entry> expired = getExpired(now);

	callingExpiredTimers_ = true;
	cancelingTimers_.clear();


	// safe to callback outside critical section
	for(std::vector<Entry>::iterator it = expired.begin();
	it != expired.end(); ++it)
	{
		it->second->run();

	}

	callingExpiredTimers_ = false;

	reset(expired, now);//重设有间隔过期的定时器，删除不要的定时器
}

/*-----------------------------------------------------*/
bool TimerQueue::insert(Timer* timer)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	
	TimerList::iterator it = timers_.begin();
	if (it == timers_.end() || when < it->first)
	{
		earliestChanged = true;
	}

	{
		std::pair<TimerList::iterator, bool> result =
			timers_.insert(Entry(when, timer));

		assert(result.second); (void)result;


	}

	{
		std::pair<ActiveTimerSet::iterator, bool> result =
			activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
		assert(result.second); (void)result;
	}

	assert(timers_.size() == activeTimers_.size());

	return earliestChanged;

}



/************************************************************************/
/* 重置过期的timer，如果有间隔的话，没有间隔的则删除                                                                     */
/************************************************************************/
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;

	for (std::vector<Entry>::const_iterator it = expired.begin();
		it != expired.end(); ++it)
	{
		ActiveTimer timer(it->second, it->second->sequence());

		//如果是重复而且没有取消
		if (it->second->repeat()
			&& cancelingTimers_.find(timer) == cancelingTimers_.end())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
			//FIXME:move to a free list 
			delete it->second; //FIXME:no delete please 

		}

	}

	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}

	if (nextExpire.valid())//跟addTimerInLoop处理类似
	{
#ifdef LINUX
		resetTimerfd(timerfd_, nextExpire);
#else 
		resetTimerWin( nextExpire );
#endif
	}
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() == activeTimers_.size());
	std::vector<Entry> expired;

	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));

	/*
	lower_bound：返回容器中第一个>=当前时间sentry的iterator位置  或者当所有元素<val 返回end
	An iterator to the the first element in the container which is not considered to go before val, or set::end if all elements are considered to go before val.
	*/
	TimerList::iterator end = timers_.lower_bound(sentry);//

	assert(end == timers_.end() || now < end->first);//);//end==timers_.end()表明全都小于，都过期了  或者now < end->first

	std::copy(timers_.begin(), end, back_inserter(expired));//[begin,end)之间的元素追加到expired末尾

	timers_.erase(timers_.begin(), end);//删除超时定时器

	for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); it++)
	{
		ActiveTimer timer(it->second, it->second->sequence());

		size_t n = activeTimers_.erase(timer);//删除超时定时器

		assert(n == 1); (void)n;
	}

	assert(timers_.size() == activeTimers_.size());
	return expired;

}

//--------------------------------------------------------------------
#ifdef WIN  //-----only Win
void TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (lpParam == NULL)
	{
		LOG_WARN << "TimerRoutine get NULL lpParam";
		return;
	}

	socket_t  sockfd = *(socket_t*)(lpParam);
	if (sockfd == INVALID_SOCKET)
	{
		LOG_WARN << "TimerRoutine get INVALID_SOCKET";
		return;
	}

	uint64_t one = 1;
	ssize_t n = sockettool::write(sockfd, &one, sizeof one);
	if (n != sizeof(one))
	{
		LOG_WARN << "TimerRoutine writes " << n << " bytes instead of 8";
	}

}

void TimerQueue::resetTimerWin(Timestamp expiration)
{
	int64_t microseconds = expiration.microSecondsSinceEpoch() -
		Timestamp::now().microSecondsSinceEpoch();

	DWORD dueTime = (DWORD)microseconds / 1000;//due:到期的，

	if (hTimer_)
	{
		if (!DeleteTimerQueueTimer(NULL, hTimer_, NULL))
		{
			LOG_ERROR << "DeleteTimerQueueTimer failed:" << getErrno() << getErrorMsg(getErrno());
			
			hTimer_ = NULL;
		
		}
	}
	/*
	BOOL WINAPI CreateTimerQueueTimer(
	_Out_    PHANDLE             phNewTimer,
	_In_opt_ HANDLE              TimerQueue,
	_In_     WAITORTIMERCALLBACK Callback,
	_In_opt_ PVOID               Parameter,
	_In_     DWORD               DueTime,
	_In_     DWORD               Period,
	_In_     ULONG               Flags
	);

	*/
	// Set a timer to call the timer routine in x seconds

	if (!CreateTimerQueueTimer(&hTimer_, NULL, (WAITORTIMERCALLBACK)TimerRoutine,
		&wakeupFd_[1], dueTime, 0, 0))
	{
		LOG_ERROR << "CreateTimerQueueTimer failed! GetLastError = " << getErrno();
	}


}
#endif //end Win
