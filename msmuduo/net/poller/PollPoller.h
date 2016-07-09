#ifndef  _PollPoller_h
#define  _PollPoller_h

#include"net/Poller.h"

NS_BEGIN

#ifdef LINUX
struct pollfd;

#else 
 
#endif

///IO Multiplexing with poll(2)

class PollPoller : public Poller
{
public:

	PollPoller(EventLoop* loop):
		Poller(loop)
	{

	}

	virtual ~PollPoller()
	{

	}

	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
	virtual void updateChannel(Channel* channel);
	virtual void removeChannel(Channel* channel);



private:
	void fillActiveChannels(int numEvents,
		ChannelList* activeChannels) const;


#ifdef LINUX
	typedef std::vector<struct pollfd> PollFdList;
#else 
	/*windows∂®“Â£∫
	typedef struct pollfd {

	SOCKET  fd;
	SHORT   events;
	SHORT   revents;

	} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;
	*/
	typedef std::vector<struct pollfd> PollFdList; 

#endif

	PollFdList pollfds_;



};


NS_END
#endif