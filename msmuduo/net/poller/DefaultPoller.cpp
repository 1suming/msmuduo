#include"stdafx.h"

#include"net/Poller.h"
#include"PollPoller.h"

#ifdef LINUX
#include"EPollPoller.h"
#endif

#include<stdlib.h>

NS_USING;

///实现Poller.h里面的方法
Poller* Poller::newDefaultPoller(EventLoop* loop)
{
#if defined NETMODEL_USE_SELECE

#elif defined NETMODEL_USE_POLL
	 
#else 
	#ifdef WIN
		#define NETMODEL_USE_POLL
		
	#else
		#define NETMODEL_USE_EPOLL
	#endif
 
 
#endif

 
}


 