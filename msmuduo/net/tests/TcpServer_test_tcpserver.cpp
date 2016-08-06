#include"msmuduo/stdafx.h"

#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"
#include"net/TcpServer.h"

#include<stdio.h>


NS_USING;


void onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		printf("onConnection(): tid=%d new connection [%s] from %s\n",
			 CurrentThread::tid(),
			conn->name().c_str(),
			conn->peerAddress().toIpPort().c_str());
	}
	else
	{
		printf("onConnection(): tid=%d connection [%s] is down\n",
			 CurrentThread::tid(),
			conn->name().c_str());
	}
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
	 
	 
	printf("onMessage(): tid=%d received %d bytes from connection [%s] at %s\n",
		 CurrentThread::tid(),
		buf->readableBytes(),//不能用%zd，否则会崩溃  z和整数转换说明符一起使用，表示一个size_t值(sizeof返回的类型)(C99)，示例：%zd,
		conn->name().c_str(),
		receiveTime.toFormattedString().c_str());
		 

	printf("onMessage():[%s]\n", buf->retrieveAllAsString().c_str());

}
int main()
{

	Logger::setLogLevel(Logger::TRACE);

	InetAddress listenAddr(9981);
	EventLoop loop;

	
	TcpServer server(&loop, listenAddr,"tcpservertest");

	server.setConnectionCallback(onConnection);

	server.setMessageCallback(onMessage);

	server.start();

	loop.loop();



}