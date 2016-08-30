#include"msmuduo/stdafx.h"
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
#ifdef LINUX 
const int Channel::kReadEvent = POLLIN | POLLPRI;
#else 
/*
如果windows下用了POLLPRI，就会包 WSAEINVAL    (10022)  错误，原因在于windows虽然定义了这个宏，但是不支持.（windows有明确的说明）
POLLPRI	Priority data may be read without blocking. This flag is not supported by the Microsoft Winsock provider.
*/
const int Channel::kReadEvent = POLLIN;
#endif
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
//这个函数被TcpConnection,Acceptor,connector等外部调用，如Acceptor
//acceptChannel_.disableAll();
//acceptChannel_.remove();

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
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))//我在windows上按ctrl+c会引发POLLHUP 和POLLERR
	{
		if (logHup_)
		{
			LOG_WARN << "Channel::handleEvent() POLLHUP";
		}
		if (closeCallback_)
			closeCallback_();

	}
	if (revents_ & POLLNVAL)
	{
		LOG_WARN << "fd = " << fd_ << " Channel::handleEvent() POLLNVAL";
	}
	if (revents_ & (POLLERR | POLLNVAL))//windows上按ctrl+c会进入这里,执行errorCallback,得到的socketerror为0
	{
		LOG_WARN << "POLLERR|POLLNVAL";
		if (errorCallback_)
			errorCallback_();
	}
	//if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	//!Importang:注意，不能用的elseif,而是用if, 
	if (revents_ & (POLLIN | POLLPRI))
	{
		if (readCallback_)
			readCallback_(receiveTime);
	}
	//用的是if，不是else if
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