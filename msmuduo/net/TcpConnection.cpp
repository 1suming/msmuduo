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
	state_(kConnecting), //state_ ��ʼ״̬
	socket_(new Socket(sockfd)), //new Socket ,TcpConnection����ʱ�Զ�����
	channel_(new Channel(loop,sockfd)), //!Imporant,������new Channel
	localAddr_(localAddr),
	peerAddr_(peerAddr),
	highWaterMark_(64*1024*1024)

{
	//!Important:ע��read ,write��error��ص�������channel
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
	assert(state_ == kDisconnected);//ʲôʱ����state�ĳ�kDisConnected
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
		/*�ж��Ƿ���loop thread �����*/
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
			�ڷ�I/O�̵߳��ã����message����һ�ݣ�bind�������ƣ�
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
	ssize_t nwrote = 0; //ע����ssize
	size_t remaining = len;
	bool faultError = false;

	//!Important:�������Ҫ���Ͽ���TcpConnection���ڲ���shared_ptr����
	//ֻҪ������TcpConnection����Ч�����Ǿ͸���state_���ж�socket������״̬
	if (state_ == kDisconnected) 
	{
		LOG_WARN << "disconnected,give up writing";
		return;
	}


	//if no thing  in output queue,try writing directly
	/*
	���ȳ���ֱ�ӷ������ݣ����һ�η�����ϾͲ�������writeCallback,���ֻ�����˲������ݣ����ʣ���
	���ݷ���outputBuffer_������ʼ��עwritableʱ�䣬�Ժ���handleWrite()�з���ʣ������ݡ�
	�����ǰoutputBuffer_�Ѿ��д����͵����ݣ���ô�Ͳ����ȳ��Է����ˣ���Ϊ��������������
	*/
	//bool isWriting() const { return events_ & kWriteEvent; }
	/*
	Ϊʲô!channel->isWriting��ȡ�Ǳ�ʾ����û�п�ʼд,ע�⿴���������ĩβ
	*/
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)//��2����������ͬʱ����������̷�
	{
 		nwrote = ::send(channel_->fd(), (char*)data,len, 0);
		//LOG_DEBUG << "---------direct send:len:"<<(int)len<<",wrote" << (int)nwrote;
		if (nwrote >= 0)
		{
			//LOG_DEBUG << "nwrote>=0";
			remaining = len - nwrote;
			if (remaining == 0 && writeCompleteCallback_)//д���˾ʹ���writeCompleteCallback
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
			&& oldLen < highWaterMark_  //��Important������Ϊ�˱���һֱ����
			&& highWaterMarkCallback_)
		{
			loop_->queueInLoop(boost::bind(highWaterMarkCallback_,
				shared_from_this(),
				oldLen + remaining));


		}


		//���δд��ĵ�outputBuffer_
		outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);

		//!Importang:������Ҫ��enableWriting
		if (!channel_->isWriting())
		{
			channel_->enableWriting();//��עwritable�¼�
			/*
			ps:ʲôʱ��POLLIN��EPOLLIN��
			epoll_ctl(epollfd, EPOLL_CTL_ADD, socket, EPOLLIN);ע�Ỻ�����ǿ��¼���������������
			EPOLLOUT��ע�Ỻ���������¼����������Ա�д�롣
			ע������һ�����������Ķ�д�������������򻺳����գ�write/read�᷵��-1��������errno=EAGAIN����epollֻ���Ļ����������ͻ������ǿ��¼�
			*/
		}


	}

}

/*
֮ǰ��sendûд������enableWriting()
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
			//!Important:һ��������ϣ�����ֹͣ�۲�writable�¼�������busy loop
			*/
			if (outputBuffer_.readableBytes() == 0)
			{
				channel_->disableWriting();

				if (writeCompleteCallback_)
				{
					loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
				}


				if (state_ == kDisconnecting) //��shutdown�������������û��shutdown�ģ���������״̬�����������״̬��shutdown.
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
		setState(kDisconnecting); //����״̬ΪkDisconnecting  ����Ҫ����֤��ĳ��ʱ��һ����ر�
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
		socket_->shutdownWrite(); //�����ر�д��shutdown(sockfd, SHUT_WR)

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
			boost::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));//bind�������shared_from_this

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
	setState(kConnected); //state �� kConnecting -> kConnected
	//channel_->tie(shared_from_this());
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);//state��kConnected -> kDisconnected
		channel_->disableAll();


		connectionCallback_(shared_from_this()); //�Ͽ�ʱ�ٵ���һ��connectionCallback
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

void TcpConnection::handleClose() //���������connectDestroy������
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