#include"stdafx.h"
#include"net/Connector.h"
#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"
#include"net/TcpServer.h"

#include<stdio.h>


NS_USING;


EventLoop* g_loop;


void connectCallback(int sockfd)
{
	printf("connected.\n");

	g_loop->quit();


}

 
int main()
{

	Logger::setLogLevel(Logger::TRACE);

	InetAddress addr("127.0.0.1",9981);
	EventLoop loop;

	g_loop = &loop;

	Connector connector (&loop, addr);

	connector.setNewConnectionCallback(connectCallback);

	connector.start();

	loop.loop();

	 
	 


}