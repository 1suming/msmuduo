#include"msmuduo/stdafx.h"
#include"Channel.h"

#include"base/Logging.h"
#include"EventLoop.h"

#include <sstream>


NS_USING;

/*
WSAPOLL �ж���POLLIN��POLLOUT
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
���windows������POLLPRI���ͻ�� WSAEINVAL    (10022)  ����ԭ������windows��Ȼ����������꣬���ǲ�֧��.��windows����ȷ��˵����
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
//���������TcpConnection,Acceptor,connector���ⲿ���ã���Acceptor
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
	revents���԰�������events��ָ�����¼�������POLLERR��POLLHUP��POLLNVAL����֮һ
	POLLERR
	����������output only��
	POLLHUP
	�Ҷϣ�output only��
	POLLNVAL
	�Ƿ�����fdδ�򿪣�output only��
	*/
	//POLLHUP����������
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))//����windows�ϰ�ctrl+c������POLLHUP ��POLLERR
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
	if (revents_ & (POLLERR | POLLNVAL))//windows�ϰ�ctrl+c���������,ִ��errorCallback,�õ���socketerrorΪ0
	{
		LOG_WARN << "POLLERR|POLLNVAL";
		if (errorCallback_)
			errorCallback_();
	}
	//if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	//!Importang:ע�⣬�����õ�elseif,������if, 
	if (revents_ & (POLLIN | POLLPRI))
	{
		if (readCallback_)
			readCallback_(receiveTime);
	}
	//�õ���if������else if
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
	//if (ev & POLLRDHUP) windows����û�����
	//	oss << "RDHUP ";
	if (ev & POLLERR)
		oss << "ERR ";
	if (ev & POLLNVAL)
		oss << "NVAL ";

	return oss.str().c_str();
}