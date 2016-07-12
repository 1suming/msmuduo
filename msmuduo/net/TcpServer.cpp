#include"stdafx.h"
#include"TcpServer.h"

#include"base/Logging.h"
#include"base/sockettool.h"

#include"Acceptor.h"
#include"EventLoop.h"


#include<boost/bind.hpp>

NS_USING;

TcpServer::TcpServer(EventLoop* loop,
			const InetAddress& listenAddr,
			const string& nameArg,
			Option option) :

	loop_(CHECK_NOTNULL(loop)),
	hostport_(listenAddr.toIpPort()),
	name_(nameArg),
	acceptor_(new Acceptor(loop,listenAddr,option==kReusePort)), //new
	//threadPool_(new EventLoopThreadPool(loop)),
	connectionCallback_(defaultConnectionCallback),
	messageCallback_(defaultMessageCallback),
	nextConnId_(1)

{
	acceptor_->setNewConnectionCallback(
		boost::bind(&TcpServer::newConnection, this, _1, _2));


}

TcpServer::~TcpServer()
{
	loop_->assertInLoopThread();
	LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

	for (ConnectionMap::iterator it(connections_.begin()); it != connections_.end(); it++)
	{
		TcpConnectionPtr conn = it->second;
		it->second.reset();//TcpConnectionPtr
		conn->getLoop()->runInLoop(
			boost::bind(&TcpConnection::connectDestroyed, conn));

		conn.reset(); //ע������ţ�����ָ�������->. conn��it->second����


	}
}

/*
void TcpServer::setThreadNum(int numThreads)
{
	assert(0 <= numThreads);
	threadPool_->setThreadNum(numThreads);
}
*/


void TcpServer::start()
{
	if (started_.getAndSet(1) == 0)
	{
		//threadPool_->start(threadInitCallback_);

		assert(!acceptor_->listenning());
		loop_->runInLoop(
			boost::bind(&Acceptor::listen, get_pointer(acceptor_)));


	}
}
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	loop_->assertInLoopThread();
	//EventLoop* ioLoop = threadPool_->getNextLoop();
	EventLoop* ioLoop = loop_;
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);

	++nextConnId_;

	string connName = name_ + buf;
	LOG_INFO << "TcpServer::newConnection [" << name_
		<< "] - new connection [" << connName
		<< "] from " << peerAddr.toIpPort();

	InetAddress localAddr(sockettool::getLocalAddr(sockfd));

	TcpConnectionPtr conn(new TcpConnection(ioLoop,
		connName,
		sockfd,
		localAddr,
		peerAddr

		));


	connections_[connName] = conn; //������conn�����ü���Ϊ2

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(
		boost::bind(&TcpServer::removeConnection, this, _1));//FIXME: unsafe

	//��ΪTcpConnection��EventLoop������������EventLoop,Ϊʲô���������Ҫ,��Ϊ�ǿ��̵߳���connectEstablished����
	ioLoop->runInLoop(
		boost::bind(&TcpConnection::connectEstablished, conn));


}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	// FIXME: unsafe
	loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));//���������һ��conn�����ü���
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
		<< "] - connection " << conn->name();
	size_t n = connections_.erase(conn->name());
	(void)n;
	assert(n == 1);
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(
		boost::bind(&TcpConnection::connectDestroyed, conn));//����bind���������ü���
}


