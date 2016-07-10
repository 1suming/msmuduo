 
#include"stdafx.h"

#include"base/lock.h"
#include"base/sockettool.h"

#include"EventLoop.h"
#include"net/Channel.h"
#include"net/Poller.h"

NS_USING;
 
namespace
{
	threadlocal EventLoop* t_loopInThisThread = 0;

	const int kPollTimeMs = 10000;

#ifdef LINUX
	int createEventfd()
	{
		int evtfd=::EVENT_FILTER_DESCRIPTOR(0,EFD_NONBLOCK | EFD_CLOEXEC);
		if (evtfd < 0)
		{
			LOG_ERROR << "Failed in eventfd";
			abort();
		}
		return evtfd;
	}

#endif

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
	threadId_(CurrentThread::tid()),

	poller_(Poller::newDefaultPoller(this)),
	//timerQueue_(new TimerQueue(this)),
#ifdef LINUX 
	wakupFd_(createEventfd()),
	wakeupChannel_(new Channe(this,wakeupFd_)),
#else 
	wsaStarupSuccess(executeWSAStartup() ),
	socketPairResult_(socketpair(AF_INET,SOCK_STREAM,0,wakeupFd_)),//执行这个会把wakeupFd_赋值
	wakeUpFd1SetNonBlocking_(make_socket_nonblocking(wakeupFd_[0])),
	wakeUpFd2SetNonBlocking_(make_socket_nonblocking(wakeupFd_[0])),

	wakeupChannel_(new Channel(this, wakeupFd_[0])), //把wakeupFd_[0]新建一个Channel

#endif
	currentActiveChannel_(NULL)

{
#ifdef WIN 
	if (socketPairResult_ !=0)
	{
		LOG_FATAL << "EventLoop constructor socketPairResult error";
	}
	assert(wakeUpFd1SetNonBlocking_ == 0 && wakeUpFd2SetNonBlocking_ == 0);
	 
#endif

 
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

	wakeupChannel_->setReadCallback(
		boost::bind(&EventLoop::handleRead, this));

	//we are always reading the wakeupfd
	wakeupChannel_->enableReading(); //channel enable和disable都会调用update(),update()会调用Loop->update，然后调用poller->update


}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
		<< " destructs in thread " << CurrentThread::tid();

	t_loopInThisThread = NULL;

	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
#ifdef LINUX
	::close(wakeupFd_);
#else 
	sockettool::close(wakeupFd_[0]);
	sockettool::close(wakeupFd_[1]);
#endif
}
 


void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;// FIXME: what if someone calls quit() before loop() ?

	LOG_TRACE << "EventLoop " << this << " start looping";


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

	// There is a chance that loop() just executes while(!quit_) and exists,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using mutex_ in both places.
	if (!isInLoopThread())
	{
		wakeup();
	}
}



void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << CurrentThread::tid();
}


/*------------------channel 相关---------------------------------------*/
void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();

	poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_)
	{
		//下面加断言是为什么
		assert(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}

	poller_->removeChannel(channel);

}
bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);

}
/*-------------run In Loop----------------------------*/
void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(cb);
	}
}
void EventLoop::queueInLoop(const Functor& cb)
{
	{
		lock_guard_t lockguard(mutex_);
		pendingFunctors_.push_back(cb);

	}
	if (!isInLoopThread() || callingPendingFunctors_) //如果不是在loopThread或者正在调用
	{
		wakeup();
	}
}

/*---------------------------wakeupFd_------------------*/

void EventLoop::wakeup()
{
	uint64_t one = 1;
#ifdef LINUX
	ssize_t n = ::send(wakeupFd_,  &one, sizeof one,0);
#else 
	ssize_t n = ::send(wakeupFd_[1], (char*)&one, sizeof one, 0);//往wakeupFd[1]写入，就可以从wakeupFd[0]读到了

#endif
	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";

	}
}
void EventLoop::handleRead()
{
	uint64_t one = 1;
#ifdef LINUX 
	ssize_t n = ::recv(wakeupFd_, &one, sizeof one, 0);
#else 
	ssize_t n = ::recv(wakeupFd_[0],(char*) &one, sizeof one,0); //从wakeupFd[0]读入
#endif

	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}

}
/*-------------------------------------------------------*/

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors; //目的是为了减少锁的竞争

	callingPendingFunctors_ = true;

	{
		lock_guard_t lockguard(mutex_);
		functors.swap(pendingFunctors_);//!Important:用到了swap 
	}

	for (size_t i = 0; i < functors.size(); i++)
	{
		functors[i]();

	}

	callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
	for (ChannelList::const_iterator it = activeChannels_.begin();
		it != activeChannels_.end(); ++it)
	{
		const Channel* ch = *it;
		LOG_TRACE << "{" << ch->reventsToString() << "} ";
	}
}