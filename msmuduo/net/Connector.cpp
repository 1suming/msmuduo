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
		connect(); //TODO:�����ε��ûᵼ��ջ���
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
	if (state_ == kConnecting) //�����������
	{
		setState(kDisconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);

	}
}


/*����������⣬
void Connector::connect()
{
int sockfd = sockettool::createNonblockingOrDie();
//   non   blockingģʽ,connect����ȵ��ɹ����������Ϸ��ء� ������select()���жϳɹ����
int ret = sockettool::connect(sockfd, serverAddr_.getSockAddrInet());

int savedErrno = (ret == 0) ? 0 : getErrno();

switch (savedErrno)
{
case 0:
case EINPROGRESS:
case EINTR:
case EISCONN:
connecting(sockfd); //����kConnecting
break;

case EAGAIN:
case EADDRINUSE:
case EADDRNOTAVAIL:
case ECONNREFUSED:
case ENETUNREACH:
case EWOULDBLOCK:// windows����#define EWOULDBLOCK     140


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
�����������ӵ��׽���
*/
void Connector::connecting(int sockfd)
{
	setState(kConnecting);
	assert(!channel_);

	channel_.reset(new Channel(loop_, sockfd));
	//  ����д�¼��ص���������д�¼�������ʾ�Ѿ����ӵ�����������һЩ����  
	channel_->setWriteCallback(boost::bind(&Connector::handleWrite, this));//FIXME:unsafe

	channel_->setErrorCallback(boost::bind(&Connector::handleError, this));

	// channel_->tie(shared_from_this()); is not working,
	// as channel_ is not managed by shared_ptr

	channel_->enableWriting(); //!!Important������������ΪenabelWriting�����ǿ��Թ�עPOLLOUT�¼�,��ִ��handleWrite
}

// �Ƴ��������¼�������  
// ÿ���������ᴴ��һ���µ��¼����������ɵ��¼��������ᱻɾ��  
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
		//// �Ƴ��������¼�������  
		int sockfd = removeAndResetChannel();
		int err = sockettool::getSocketError(sockfd);
		if (err)//// �д���������ô������  
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
		else // // û�д���Ҳ�����Լ����Լ������ӣ���ʾ���ӳɹ�  
		{
			setState(kConnected);//��Important:������Ű�kConnecting���kConnected
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

		/*��shared_from_this()���� bad_weak_ptr����ʱ�����ԭ��
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