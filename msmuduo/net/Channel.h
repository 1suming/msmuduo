#ifndef _Channel_H
#define _Channel_H


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include"msmuduo/base/Timestamp.h"

NS_BEGIN

class EventLoop;

/*
Channel 就是一个EventHandler

A selectable I/o channel
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd

*/
class Channel : boost::noncopyable
{
public:
	typedef boost::function<void()> EventCallback;
	typedef boost::function<void(Timestamp)> ReadEventCallback;

	Channel(EventLoop* loop, int fd):
		loop_(loop),
		fd_(fd),
		events_(0),
		revents_(0),
		index_(-1),//-1表示还没有加入到poller
		logHup_(false),

		eventHandling_(false)

	{

	}
	~Channel();

	void handleEvent(Timestamp receiveTime);

	void setReadCallback(const ReadEventCallback& cb)
	{
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback& cb)
	{
		writeCallback_ = cb;
	}
	void setCloseCallback(const EventCallback& cb)
	{
		closeCallback_ = cb;
	}
	void setErrorCallback(const EventCallback& cb)
	{
		errorCallback_ = cb;
	}

	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; }
	
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	//!Importang:channel的enable和disable都会调用update() {loop_->updateChannel(this);}
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	/*
	 warning C4800: “const int”: 将值强制为布尔值“true”或“false”(性能警告)
	 */
#pragma warning(disable:4800)
	bool isWriting() const { return events_ & kWriteEvent; }
	bool isReading() const { return events_ & kReadEvent; }
#pragma  warning(default:4800)
	//for Poller
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }


	//for debug
	string reventsToString() const;
	string eventsToString() const;

	void doNotLogHup() { logHup_ = false; }

	EventLoop* ownerLoop() { return loop_; }
	void remove();



private:
	static string eventsToString(int fd, int ev);

	void update();
	//void handleEventWithGuard(Timestamp receiveTime);




	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;


	EventLoop* loop_;
	const int fd_;
	int events_;
	int revents_; //it's the received event type of epoll or poll
	int index_;//used by Poller
	bool logHup_; //POLLHUP

	bool eventHandling_; //是否正在进行handleEvent操作





	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;



};


NS_END
#endif