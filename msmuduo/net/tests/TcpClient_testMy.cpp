#include"msmuduo/stdafx.h"
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

 

void connectCallback( const TcpConnectionPtr& conn)
{
	LOG_INFO << "-----------connectionCallback";
	string msg = "a";

	conn->send(msg);
 

}

void messageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	LOG_DEBUG << "---mesageCallback";
	string str = buf->retrieveAllAsString();
	printf("%s\n", str.c_str());
	conn->send(str);
}


int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::TRACE);
	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 9981); // no such server
	TcpClient client(&loop, serverAddr, "TcpClient");
 	client.connect();

	client.setConnectionCallback(connectCallback);
	client.setMessageCallback(messageCallback);
	loop.loop();
}