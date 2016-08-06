#include"msmuduo/stdafx.h"
#include"Connector.h"
#include"net/EventLoop.h"
#include"base/sockettool.h"
#include"base/Logging.h"
#include"net/Channel.h"


#include<boost/bind.hpp>

NS_USING;
 

void Connector::start()
{
	connect_ = true;
	loop_->runInLoop(boost::bind(
		&Connector::startInLoop, this));

}

void Connector::startInLoop()
{
	loop_->assertInLoopThread();
	assert(state_ == kDisconnected);
	if (connect_)
	{
		connect(); //TODO:这里多次调用会导致栈溢出
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}
void Connector::stop()
{
	connect_ = false;
	loop_->queueInLoop(boost::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
	// FIXME: cancel timer
}

void Connector::stopInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnecting) //如果正在连接
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);

	}
}


/*下面的有问题，
void Connector::connect()
{
int sockfd = sockettool::createNonblockingOrDie();
//   non   blocking模式,connect不会等到成功，而是马上返回。 可以用select()来判断成功与否。
int ret = sockettool::connect(sockfd, serverAddr_.getSockAddrInet());

int savedErrno = (ret == 0) ? 0 : getErrno();

switch (savedErrno)
{
case 0:
case EINPROGRESS:
case EINTR:
case EISCONN:
connecting(sockfd); //设置kConnecting
break;

case EAGAIN:
case EADDRINUSE:
case EADDRNOTAVAIL:
case ECONNREFUSED:
case ENETUNREACH:
case EWOULDBLOCK:// windows定义#define EWOULDBLOCK     140


//#define WSAEWOULDBLOCK                   10035L 
#ifdef WIN 
	case WSAEWOULDBLOCK:
	case WSAEINPROGRESS: //10036
#endif
		retry(sockfd); //!!Retry ,EAGAIN 
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
		sockettool::close(sockfd);
		break;

	default:
		LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
		sockettool::close(sockfd);
		// connectErrorCallback_();
		break;
	}


}
*/
void Connector::connect()
{
	int sockfd = sockettool::createNonblockingOrDie();
	int ret = sockettool::connect(sockfd, serverAddr_.getSockAddrInet());
	int savedErrno = (ret == 0) ? 0 : errno;
	LOG_DEBUG << "savedErrno:" << savedErrno << "->" << getErrorMsg(savedErrno);
	switch (savedErrno)
	{
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(sockfd);
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry(sockfd);
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
		sockettool::close(sockfd);
		break;

	default:
		LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
		sockettool::close(sockfd);
		// connectErrorCallback_();
		break;
	}
}

void Connector::restart()
{
	loop_->assertInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
	startInLoop();

}

/*
处理正在连接的套接字
*/
void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	assert(!channel_);

	channel_.reset(new Channel(loop_, sockfd));
	//  设置写事件回调函数（有写事件发生表示已经连接到服务器或者一些错误）  
	channel_->setWriteCallback(boost::bind(&Connector::handleWrite, this));//FIXME:unsafe

	channel_->setErrorCallback(boost::bind(&Connector::handleError, this));

	// channel_->tie(shared_from_this()); is not working,
	// as channel_ is not managed by shared_ptr

	channel_->enableWriting(); //!!Important，把设置设置为enabelWriting，我们可以关注POLLOUT事件,会执行handleWrite
}

// 移除并重置事件处理器  
// 每次重连都会创建一个新的事件处理器，旧的事件处理器会被删除  
int Connector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	// Can't reset channel_ here, because we are inside Channel::handleEvent
	loop_->queueInLoop(boost::bind(&Connector::resetChannel, this)); // FIXME: unsafe
	return sockfd;
}

void Connector::resetChannel()
{
	channel_.reset();
}


void Connector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite " << state_;

	if (state_ == kConnecting)
	{
		//// 移除并重置事件处理器  
		int sockfd = removeAndResetChannel();
		int err = sockettool::getSocketError(sockfd);
		if (err)//// 有错误发生，那么就重连  
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = "
				<< err << " " << getErrorMsg(err);

			retry(sockfd);
		}
		else if (sockettool::isSelfConnect(sockfd))
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry(sockfd);
		}
		else // // 没有错误、也不是自己对自己的连接，表示连接成功  
		{
			setState(kConnected);//！Important:在这里才把kConnecting变成kConnected
			if (connect_)
			{
				newConnectionCallback_(sockfd);

			}
			else
			{
				sockettool::close(sockfd);
			}
		}
	}
	else
	{
		//what happened 
		assert(state_ == kDisconnected);
	}

}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError state=" << state_;
	if (state_ == kConnecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockettool::getSocketError(sockfd);
		LOG_TRACE << "SO_ERROR = " << err << " " << getErrorMsg(err);
		retry(sockfd);
	}
}



/*--------------------------------------------------*/
void Connector::retry(int sockfd)
{
	sockettool::close(sockfd);
	setState(kDisconnected);

	if (connect_)
	{
		LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
			<< " in " << retryDelayMs_ << " milliseconds. ";

		/*用shared_from_this()会有 bad_weak_ptr，暂时不清楚原因
		loop_->runAfter(retryDelayMs_ / 1000.0,
		boost::bind(&Connector::startInLoop, shared_from_this()));
		*/
		loop_->runAfter(retryDelayMs_ / 1000.0,
			boost::bind(&Connector::startInLoop, this));
		//retryDelayMs_ = (std::min)(retryDelayMs_ * 2, kMaxRetryDelayMs);

	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}