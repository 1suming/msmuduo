#include"stdafx.h"
#include"TcpClient.h"
#include"base/Logging.h"
#include"base/sockettool.h"
#include"net/EventLoop.h"
#include"net/Channel.h"


NS_BEGIN
namespace detail
{
	void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
	{
		loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));

	}
	void removeConnector(const ConnectorPtr& connector)
	{
		//connector->
	}


}
NS_END

NS_USING;


TcpClient::TcpClient(EventLoop* loop,
	const InetAddress& serverAddr,
	const string& nameArg)
	: loop_(CHECK_NOTNULL(loop)),
	connector_(new Connector(loop, serverAddr)),
	name_(nameArg),
	connectionCallback_(defaultConnectionCallback),
	messageCallback_(defaultMessageCallback),
	retry_(false),
	connect_(true),
	nextConnId_(1)
{
	connector_->setNewConnectionCallback(
		boost::bind(&TcpClient::newConnection, this, _1));
	// FIXME setConnectFailedCallback
	LOG_INFO << "TcpClient::TcpClient[" << name_
		<< "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
	LOG_INFO << "TcpClient::~TcpClient[" << name_
		<< "] - connector " << get_pointer(connector_);
	TcpConnectionPtr conn;
	bool unique = false;
	{
		lock_guard_t lock(mutex_);
		/*http://www.cplusplus.com/reference/memory/shared_ptr/unique/
		bool unique() const noexcept;
		Check if unique
		Returns whether the shared_ptr object does not share ownership over its pointer with other shared_ptr objects (i.e., it is unique).

		Empty pointers are never unique (as they do not own any pointers).

		Unique shared_ptr objects are responsible to delete their managed object if they release this ownership (see the destructor).
		This function shall return the same as (use_count()==1), although it may do so in a more efficient way.

		*/
		unique = connection_.unique();
		conn = connection_;
	}
	if (conn)
	{
		assert(loop_ == conn->getLoop());
		// FIXME: not 100% safe, if we are in different thread
		CloseCallback cb = boost::bind(&detail::removeConnection, loop_, _1);
		loop_->runInLoop(
			boost::bind(&TcpConnection::setCloseCallback, conn, cb));
		if (unique)
		{
			conn->forceClose();
		}
	}
	else
	{
		connector_->stop();
		// FIXME: HACK
		loop_->runAfter(1, boost::bind(&detail::removeConnector, connector_));
	}
}

void TcpClient::connect()
{
	// FIXME: check state
	LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
		<< connector_->serverAddress().toIpPort();
	connect_ = true;
	connector_->start();
}

void TcpClient::disconnect()
{
	connect_ = false;

	{
		lock_guard_t lock(mutex_);
		if (connection_)
		{
			connection_->shutdown();
		}
	}
}

void TcpClient::stop()
{
	connect_ = false;
	connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
	loop_->assertInLoopThread();
	InetAddress peerAddr(sockettool::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
	++nextConnId_;
	string connName = name_ + buf;

	InetAddress localAddr(sockettool::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	TcpConnectionPtr conn(new TcpConnection(loop_,
		connName,
		sockfd,
		localAddr,
		peerAddr));

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(
		boost::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
	{
		lock_guard_t lock(mutex_);
		connection_ = conn;
	}
	conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	assert(loop_ == conn->getLoop());

	{
		lock_guard_t lock(mutex_);
		assert(connection_ == conn);
		connection_.reset();
	}

	loop_->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
	if (retry_ && connect_)
	{
		LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
			<< connector_->serverAddress().toIpPort();
		connector_->restart();
	}
}
