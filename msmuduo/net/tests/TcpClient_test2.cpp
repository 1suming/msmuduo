#include"stdafx.h"
#include"net/Connector.h"
#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"
#include"net/TcpServer.h"
#include"net/TcpClient.h"
#include"base/Logging.h"

#include<stdio.h>

NS_USING;

//// TcpClient destructs when TcpConnection is connected but unique.


void threadFunc(EventLoop* loop)
{
	InetAddress serverAddr("127.0.0.1", 9981);
	TcpClient client(loop, serverAddr, "TcpClient");

	client.connect();

	sleep(1);
	// client destructs when connected.
}

int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::DEBUG);

	EventLoop loop;
	loop.runAfter(3.0, boost::bind(&EventLoop::quit, &loop));
	Thread thr(boost::bind(threadFunc, &loop));
	thr.start();
	loop.loop();
}