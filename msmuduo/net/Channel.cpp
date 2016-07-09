#include"stdafx.h"
#include"Channel.h"

#include"base/Logging.h"
#include"EventLoop.h"

#include <sstream>


NS_USING;

/*
WSAPOLL 有定义POLLIN和POLLOUT
 Event flag definitions for WSAPoll().  

#define POLLRDNORM  0x0100
#define POLLRDBAND  0x0200
#define POLLIN      (POLLRDNORM | POLLRDBAND)
#define POLLPRI     0x0400

#define POLLWRNORM  0x0010
#define POLLOUT     (POLLWRNORM)
#define POLLWRBAND  0x0020

#define POLLERR     0x0001
#define POLLHUP     0x0002
#define POLLNVAL    0x0004

*/

const int Channel::kNoneEvent = 0;

const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
 

Channel::~Channel()
{
	assert(!eventHandling_);
	//assert(!addedToLoop_);
	if (loop_->isInLoopThread())
	{
		assert(!loop_->hasChannel(this));
	}


}

void Channel::update()
{
 	loop_->updateChannel(this);

}

void Channel::remove()
{
	assert(isNoneEvent());
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	/*
	revents可以包含任意events中指定的事件，或是POLLERR、POLLHUP、POLLNVAL三者之一
	POLLERR
	错误条件（output only）
	POLLHUP
	挂断（output only）
	POLLNVAL
	非法请求：fd未打开（output only）
	*/
	//POLLHUP描述符挂起
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
	{
		if (logHup_)
		{
			LOG_WARN << "Channel::handleEvent)( POLLHUP";
		}
		if (closeCallback_)
			closeCallback_();

	}
	if (revents_ & POLLNVAL)
	{
		LOG_WARN << "fd = " << fd_ << " Channel::handleEvent() POLLNVAL";
	}
	if (revents_ & (POLLERR | POLLNVAL)) 
	{
		if (errorCallback_)
			errorCallback_();
	}
	//if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	if (revents_ & (POLLIN | POLLPRI))
	{
		if (readCallback_)
			readCallback_(receiveTime);
	}
	if (revents_ & POLLOUT)
	{
		if (writeCallback_)
			writeCallback_();
	}

	eventHandling_ = false;


}

string Channel::reventsToString() const
{
	return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
	return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & POLLIN)
		oss << "IN ";
	if (ev & POLLPRI)
		oss << "PRI ";
	if (ev & POLLOUT)
		oss << "OUT ";
	if (ev & POLLHUP)
		oss << "HUP ";
	//if (ev & POLLRDHUP) windows上面没有这个
	//	oss << "RDHUP ";
	if (ev & POLLERR)
		oss << "ERR ";
	if (ev & POLLNVAL)
		oss << "NVAL ";

	return oss.str().c_str();
}