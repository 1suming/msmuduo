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
		buf->readableBytes(),
		conn->name().c_str(),
		receiveTime.toFormattedString().c_str());

	string str(buf->peek(), buf->readableBytes());
	printf("str:%s\n", str.c_str());

	conn->send(buf->retrieveAllAsString());

}
int main()
{

	Logger::setLogLevel(Logger::TRACE);

	InetAddress listenAddr(9981);
	EventLoop loop;


	TcpServer server(&loop, listenAddr, "tcpservertest");

	server.setConnectionCallback(onConnection);

	server.setMessageCallback(onMessage);

	server.setThreadNum(3);


	server.start();

	loop.loop();



}