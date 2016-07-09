#include"stdafx.h"
#include"PollPoller.h"
#include"base/sockettool.h"
#include"base/Logging.h"
#include"net/Channel.h"

#ifdef LINUX
#inlcude<poll.h>
#else 
#endif

NS_USING;

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels) //vector<Channel*>
{
	//XXX pollfds_ shouldn't change
#ifdef LINUX
	int numEvents = ::pollfd(&*pollfds_.begin(),  pollfds_.size(), timeoutMs);
#else 
	/*
	msdn WSAPOLL:https://msdn.microsoft.com/en-us/library/ms741669(VS.85).aspx
	Flag	Description
	POLLPRI	Priority data may be read without blocking. This flag is not supported by the Microsoft Winsock provider.
	POLLRDBAND	Priority band (out-of-band) data may be read without blocking.
	POLLRDNORM	Normal data may be read without blocking.
	POLLWRNORM	Normal data may be written without blocking.
	
	*/
 	int numEvents = ::WSAPoll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);

#endif
	int savedError = getErrno();

	Timestamp now(Timestamp::now());
	if (numEvents > 0)
	{
		LOG_TRACE << numEvents << " events happended";
		fillActiveChannels(numEvents, activeChannels);
	}
	else if (numEvents == 0)
	{
		LOG_TRACE << " nothing happended ";
	}
	else
	{
		if (savedError != EINTR)
		{
			errno = savedError;
			LOG_ERROR << "PollPoller::poll()" << savedError << getErrorMsg(savedError);
		}
	}
	return now;

}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
 	for (PollFdList::const_iterator pfd = pollfds_.begin();
		pfd != pollfds_.end() && numEvents>0; ++pfd)
	{
		if (pfd->revents > 0)
		{
			--numEvents;//每找到一个就递减numEvents，找完了就退出，不必循环所有的
			ChannelMap::const_iterator ch = channels_.find(pfd->fd);
			assert(ch != channels_.end());
			Channel* channel = ch->second;
			assert(channel->fd() == pfd->fd);
			channel->set_revents(pfd->revents);//set events，处理事件在Channel中handleEvent进行
			//pfd->revent=0
			activeChannels->push_back(channel);
 
		}
	}

}

//这个函数由EventLoop调用
void PollPoller::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd() << " events= " << channel->events();
	if (channel->index() < 0)
	{
		//a new one, add to pollfds
		assert(channels_.find(channel->fd()) == channels_.end());
#ifdef LINUX
		struct pollfd pfd;
#else 
		struct pollfd pfd;
#endif
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		pollfds_.push_back(pfd);// 增加pollfd
		int idx = static_cast<int>(pollfds_.size()) - 1;
		channel->set_index(idx);
		channels_[pfd.fd] = channel;


	}
	else
	{
		//update existing one 
		assert(channels_.find(channel->fd()) != channels_.end());
		assert(channels_[channel->fd()] == channel);

		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

		struct pollfd& pfd = pollfds_[idx];
		assert( pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);

		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent())
		{
			//ignore this pollfd
			pfd.fd = -channel->fd() - 1;//为什么要减1

		}

	}

}

void PollPoller::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	LOG_TRACE << "fd = " << channel->fd();
	assert(channels_.find(channel->fd()) != channels_.end());
	assert(channels_[channel->fd()] == channel);

	//只有为isNoneEvent才删除
	assert(channel->isNoneEvent());

	int idx = channel->index();
	assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

	const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
	assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());

	size_t n = channels_.erase(channel->fd());
	assert(n == 1); (void)n;

	if (implicit_cast<size_t>(idx) == pollfds_.size()-1)
	{
		pollfds_.pop_back();
	}
	else
	{
		int channelAtEnd = pollfds_.back().fd;
		//迭代器交换
		iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
		if (channelAtEnd < 0)
		{
			channelAtEnd = -channelAtEnd - 1;//如果是负数还原为原来的
		}

		channels_[channelAtEnd]->set_index(idx);//因为channelAtEnd表示最后一个描述符,set_index用来设置最后一个的index为channel的idx
		pollfds_.pop_back();

	}
}