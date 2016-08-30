#ifndef __Poller_h
#define __Poller_h

#include<boost/noncopyable.hpp>
#include"base/Timestamp.h"
#include"net/EventLoop.h"

NS_BEGIN

class Channel;

class Poller : boost::noncopyable
{
public:
	typedef std::vector<Channel*> ChannelList;

	Poller(EventLoop* loop):
		ownerLoop_(loop)
	{

	}
	virtual ~Poller()
	{

	}

	///polls the I/O events 
	///Must be called in the loop thread
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;


	///Changes the interested I/o events
	///must be called in the loop thread
	virtual void updateChannel(Channel* channel) = 0;

	///remove the channel,when it destructs
	///Must be called in the loop thread
	virtual void removeChannel(Channel* channel) = 0;

	virtual bool hasChannel(Channel* channel) const;

	static Poller* newDefaultPoller(EventLoop* loop);


	void assertInLoopThread() const
	{
		ownerLoop_->assertInLoopThread();
	}

	inline int getChannelMapSize()
	{
		return channels_.size();
	}

protected:
	typedef std::map<int, Channel*> ChannelMap;//Channel->fd()作为key
	ChannelMap channels_; //保存fd到Channel的映射

private:
	EventLoop* ownerLoop_;

};




NS_END

#endif