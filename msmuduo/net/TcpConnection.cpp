#include"msmuduo/stdafx.h"
#include"msmuduo/net/TcpConnection.h"


#include"msmuduo/base/Logging.h"
#include"msmuduo/base/sockettool.h"

#include"msmuduo/net/EventLoop.h"
#include"msmuduo/net/Socket.h"


#include<boost/bind.hpp>



NS_BEGIN
void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
	// do not call conn->forceClose(), because some users want to register message callback only.
}

void  defaultMessageCallback(const TcpConnectionPtr&,
	Buffer* buf,
	Timestamp)
{
	buf->retrieveAll();
}
NS_END


NS_USING;

TcpConnection::TcpConnection(EventLoop* loop,
							const string& nameArg,
							int sockfd,
							const InetAddress& localAddr,
							const InetAddress& peerAddr):
	loop_(CHECK_NOTNULL(loop)),
	name_(nameArg),
	state_(kConnecting), //state_ 初始状态
	socket_(new Socket(sockfd)), //new Socket ,TcpConnection析构时自动析构
	channel_(new Channel(loop,sockfd)), //!Imporant,在这里new Channel
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	highWaterMark_(64*1024*1024)

{
	//!Important:注册read ,write，error会回调函数给channel
	channel_->setReadCallback(
		boost::bind(&TcpConnection::handleRead, this, _1));
	channel_->setWriteCallback(
		boost::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(
		boost::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(
		boost::bind(&TcpConnection::handleError, this));



	LOG_OK<< "TcpConnection::Constructor[" << name_ << "] at " << this
		<< " fd=" << sockfd;

	socket_->setKeepAlive(true);

}

TcpConnection::~TcpConnection()
{
	LOG_OK << "TcpConnection::Destructor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd()
		<< " state=" << state_;
	assert(state_ == kDisconnected);//什么时候会把state改成kDisConnected
}


/*
bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const
{
char buf[1024];
buf[0] = '\0';
socket_->getTcpInfoString(buf, sizeof buf);
return buf;
}

*/

void TcpConnection::send(const void* data, int len)
{
	send(StringPiece(static_cast<const char*>(data), len));
}
void TcpConnection::send(const StringPiece& message)
{
	if (state_ == kConnected)
	{
		/*判断是否在loop thread 里调用*/
		if (loop_->isInLoopThread())
		{
			sendInLoop(message);
		}
		else
		{
			loop_->runInLoop(
				boost::bind(&TcpConnection::sendInLoop,
				this,
				message.as_string()));//FIXME://std::forward<string>(message)


		}
	}
}

//FIXME: efficiency!!
void TcpConnection::send(Buffer* buf)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInLoop(buf->peek(), buf->readableBytes());
			buf->retrieveAll();
		}
		else
		{
			/*
			在非I/O线程调用，会把message复制一份（bind函数复制）
			*/
			loop_->runInLoop(
				boost::bind(&TcpConnection::sendInLoop,
				this,
				buf->retrieveAllAsString()));////std::forward<string>(message)
		}
	}
}

void TcpConnection::sendInLoop(const StringPiece& message)
{
	sendInLoop(message.data(), message.size());
}
void TcpConnection::sendInLoop(const void* data, size_t len)
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0; //注意是ssize
	size_t remaining = len;
	bool faultError = false;

	//!Important:这里很重要，断开后TcpConnection由于采用shared_ptr管理，
	//只要有引用TcpConnection就有效，我们就根据state_来判断socket的连接状态
	if (state_ == kDisconnected) 
	{
		LOG_WARN << "disconnected,give up writing";
		return;
	}


	//if no thing  in output queue,try writing directly
	/*
	会先尝试直接发送数据，如果一次发送完毕就不会启用writeCallback,如果只发送了部分数据，则把剩余的
	数据放入outputBuffer_，并开始关注writable时间，以后在handleWrite()中发送剩余的数据。
	如果当前outputBuffer_已经有待发送的数据，那么就不能先尝试发送了，因为这会造成数据乱序。
	*/
	//bool isWriting() const { return events_ & kWriteEvent; }
	/*
	为什么!channel->isWriting，取非表示我们没有开始写,注意看这个函数的末尾
	*/
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)//这2个条件必须同时满足才能立刻发
	{
 		nwrote = ::send(channel_->fd(), (char*)data,len, 0);
		//LOG_DEBUG << "---------direct send:len:"<<(int)len<<",wrote" << (int)nwrote;
		if (nwrote >= 0)
		{
			//LOG_DEBUG << "nwrote>=0";
			remaining = len - nwrote;
			if (remaining == 0 && writeCompleteCallback_)//写完了就触发writeCompleteCallback
			{
				loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));

			}
		}
		else //nwrote < 0
		{
			//LOG_DEBUG<< "nwote<0";
			nwrote = 0;
			if (errno != EWOULDBLOCK)
			{
				LOG_ERROR << "TcpConnection::SendInLoop";
				if (errno == EPIPE || errno == ECONNRESET) //FIXME: any other?
				{
					faultError = true;
				}
			}
		}
	}

	assert((remaining + nwrote) == len);
	assert(remaining <= len);
	if (!faultError && remaining>0)
	{
		size_t oldLen = outputBuffer_.readableBytes();

		if (oldLen + remaining >= highWaterMark_
			&& oldLen < highWaterMark_  //！Important这里是为了避免一直触发
			&& highWaterMarkCallback_)
		{
			loop_->queueInLoop(boost::bind(highWaterMarkCallback_,
				shared_from_this(),
				oldLen + remaining));


		}


		//添加未写完的到outputBuffer_
		outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);

		//!Importang:至关重要，enableWriting
		if (!channel_->isWriting())
		{
			channel_->enableWriting();//关注writable事件
			/*
			ps:什么时候POLLIN或EPOLLIN，
			epoll_ctl(epollfd, EPOLL_CTL_ADD, socket, EPOLLIN);注册缓冲区非空事件，即有数据流入
			EPOLLOUT，注册缓冲区非满事件，即流可以被写入。
			注：当对一个非阻塞流的读写发生缓冲区满或缓冲区空，write/read会返回-1，并设置errno=EAGAIN。而epoll只关心缓冲区非满和缓冲区非空事件
			*/
		}


	}

}

/*
之前的send没写完会调用enableWriting()
*/
void TcpConnection::handleWrite()
{
	loop_->assertInLoopThread();
	if (channel_->isWriting())
	{
		ssize_t n = ::send(channel_->fd(),
			outputBuffer_.peek(),
			outputBuffer_.readableBytes(),
			0);


		if (n > 0)
		{
			outputBuffer_.retrieve(n);
			/*
			//!Important:一旦发送完毕，立刻停止观察writable事件，避免busy loop
			*/
			if (outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();

				if (writeCompleteCallback_)
				{
					loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
				}


				if (state_ == kDisconnecting) //在shutdown里面如果正在是没有shutdown的，但会设置状态，这里如果有状态则shutdown.
				{
					shutdownInLoop();
				}
			}
		}
		else
		{
			LOG_ERROR << "TcpConnection::handleWrite";
		}

	}
	else
	{
		LOG_TRACE << "Connection fd = " << channel_->fd()
			<< " is down, no more writing";
	}
}



void TcpConnection::shutdown()
{
	//FIXME:use compare and swap
	if (state_ == kConnected)
	{
		setState(kDisconnecting); //设置状态为kDisconnecting  很重要，保证在某个时刻一定会关闭
		//FIXME:use shared_from_this
		loop_->runInLoop(
			boost::bind(&TcpConnection::shutdownInLoop, this));
	}
}
void TcpConnection::shutdownInLoop()
{
	loop_->assertInLoopThread();
	if (!channel_->isWriting())
	{
		//we are not writing
		socket_->shutdownWrite(); //仅仅关闭写端shutdown(sockfd, SHUT_WR)

	}
}
// void TcpConnection::shutdownAndForceCloseAfter(double seconds)
// {
//   // FIXME: use compare and swap
//   if (state_ == kConnected)
//   {
//     setState(kDisconnecting);
//     loop_->runInLoop(boost::bind(&TcpConnection::shutdownAndForceCloseInLoop, this, seconds));
//   }
// }

// void TcpConnection::shutdownAndForceCloseInLoop(double seconds)
// {
//   loop_->assertInLoopThread();
//   if (!channel_->isWriting())
//   {
//     // we are not writing
//     socket_->shutdownWrite();
//   }
//   loop_->runAfter(
//       seconds,
//       makeWeakCallback(shared_from_this(),
//                        &TcpConnection::forceCloseInLoop));
// }


void TcpConnection::forceClose()
{
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		setState(kDisconnecting);
		loop_->queueInLoop(
			boost::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));//bind后面可用shared_from_this

	}
}

void TcpConnection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected || state_ == kDisconnecting)
	{
		//as if we receive 0 byte in handleRead()
		handleClose();
	}
}

void TcpConnection::setTcpNoDelay(bool on)
{
	socket_->setTcpNoDelay(on);
}

const char* TcpConnection::stateToString() const
{
	switch (state_)
	{
	case kDisconnected:
		return "kDisconnected";
	case kConnecting:
		return "kConnecting";
	case kConnected:
		return "kConnected";
	case kDisconnecting:
		return "kDisconnecting";
	default:
		return "unknown state";
	}
}



/*-------------------------*/
void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected); //state 从 kConnecting -> kConnected
	//channel_->tie(shared_from_this());
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);//state从kConnected -> kDisconnected
		channel_->disableAll();


		connectionCallback_(shared_from_this()); //断开时再调用一次connectionCallback
	}

	channel_->remove();//!Important:
}

/*----------------------------handle ------------------------------*/

void TcpConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	if (n > 0)
	{
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);

	}
	else if (n == 0)
	{
 		handleClose();
	}
	else
	{
		errno = savedErrno;
		LOG_ERROR << "TcpConnection::handleRead";
		handleError();
	}

}

void TcpConnection::handleClose() //这个函数跟connectDestroy有类似
{
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd() << " state = " << state_;

	assert(state_ == kConnected || state_ == kDisconnecting);
	// we don't close fd, leave it to dtor, so we can find leaks easily.   

	setState(kDisconnected);
	channel_->disableAll();


	TcpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	//must be the last line
	closeCallback_(guardThis);

}

void TcpConnection::handleError()
{
	int err = sockettool::getSocketError(channel_->fd());
	LOG_ERROR << "TcpConnection::handleError [" << name_
		<< "] - SO_ERROR = " << err << " " << getErrorMsg(err);
}