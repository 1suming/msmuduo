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

//// TcpClient::stop() called in the same iteration of IO event

TcpClient* g_client;

void timeout()
{
	LOG_INFO << "timeout";
	g_client->stop();
}
int main(int argc, char* argv[])
{
	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 2); // no such server
	TcpClient client(&loop, serverAddr, "TcpClient");
	g_client = &client;
	loop.runAfter(0.0, timeout);
	loop.runAfter(1.0, boost::bind(&EventLoop::quit, &loop));
	client.connect();
 
	loop.loop();
}