#include"msmuduo/stdafx.h"
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

	/*
	不能用
	 Connector connector (&loop, addr);
	 因为在主函数main中，D的实例是在栈上构造，没有使用boost::shared_ptr<D> 的构造方式，
	 所以boost::enable_shared_from_this<D>中的weak_ptr所指的函数对象也就没有被赋值，
	 在调用d.func()中使用shared_from_this()函数时
	 http://blog.csdn.net/yockie/article/details/40213331
	
	loop_->runAfter(retryDelayMs_ / 1000.0,
	boost::bind(&Connector::startInLoop, shared_from_this()));
	
	*/
	shared_ptr<Connector> pConnector(new Connector(&loop, addr));

	pConnector->setNewConnectionCallback( connectCallback );

	pConnector->start();

	loop.loop();

	 
	 


}