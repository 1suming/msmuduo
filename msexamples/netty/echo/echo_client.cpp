#include"stdafx.h"
#include "msmuduo/net/TcpClient.h"

 
#include "msmuduo/base/Logging.h"
#include "msmuduo/base/Thread.h"
#include "msmuduo/net/EventLoop.h"
#include "msmuduo/net/InetAddress.h"

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
 

using namespace ms;

class EchoClient : boost::noncopyable
{
public:
	EchoClient(EventLoop* loop, const InetAddress& listenAddr, int size)
		: loop_(loop),
		client_(loop, listenAddr, "EchoClient"),
		message_(size, 'H')
	{
		client_.setConnectionCallback(
			boost::bind(&EchoClient::onConnection, this, _1));
		client_.setMessageCallback(
			boost::bind(&EchoClient::onMessage, this, _1, _2, _3));
		//client_.enableRetry();
	}
	void connect()
	{
		client_.connect();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_TRACE << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			conn->setTcpNoDelay(true);
			conn->send(message_);
		}
		else
		{
			loop_->quit();
		}
	}

	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
	{
		string message = buf->retrieveAllAsString();
		LOG_INFO << message;
		conn->send(message);
	}


private:
	EventLoop* loop_;
	TcpClient client_;
	string message_;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();

	uint16_t port = 9981;
	string ip = "127.0.0.1";
	int size = 10;

	if (argc==1)
	{
		printf("Usage: %s ip port [msg_size]\n", argv[0]);
		return -1;
	}
	else if (argc == 2)
	{
		port = atoi(argv[1]);

	}
	else if (argc==3)
	{
		ip = argv[1];
		port = atoi(argv[2]);

	}
	else if (argc >3)
	{
		ip = argv[1];
		port = atoi(argv[2]);

		size = atoi(argv[3]);
	}
	 
	EventLoop loop;

	InetAddress serverAddr(ip, port);


	EchoClient client(&loop, serverAddr, size);
	client.connect();
	loop.loop();
 
}
