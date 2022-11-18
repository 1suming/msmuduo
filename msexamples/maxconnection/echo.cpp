#include"echo.h"
#include<boost/bind.hpp>


#include<msmuduo/base/Logging.h>

NS_USING;

EchoServer::EchoServer(EventLoop* loop,
	const InetAddress& listenAddr,
	int maxConnections) :
	server_(loop, listenAddr, "EchoServer"),
	numConnected_(0),
	kMaxConnections_(maxConnections)
{
	/*2022-02-12��Ҫ��std::bind����Ȼ����ʾ�Ҳ�����Ҫ��boost
	server_.setConnectionCallback(
		std::bind(&EchoServer::onConnection, this, _1));

	server_.setMessageCallback(
		std::bind(&EchoServer::onMessage, this, _1, _2, _3));
		*/
	server_.setConnectionCallback(
		boost::bind(&EchoServer::onConnection, this, _1));

	server_.setMessageCallback(
		boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
	server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			++numConnected_;
			if (numConnected_ > kMaxConnections_)
			{
				conn->shutdown();
				conn->forceCloseWithDelay(3.0); //round trip of the whole internet

			}
		}
		else
		{
			numConnected_--;

		}

		LOG_INFO << "numConnected = " << numConnected_;

}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
	Buffer* buf,
	Timestamp time)
{
	string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time.toString();
	conn->send(msg);
}