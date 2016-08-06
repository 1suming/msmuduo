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
	������
	 Connector connector (&loop, addr);
	 ��Ϊ��������main�У�D��ʵ������ջ�Ϲ��죬û��ʹ��boost::shared_ptr<D> �Ĺ��췽ʽ��
	 ����boost::enable_shared_from_this<D>�е�weak_ptr��ָ�ĺ�������Ҳ��û�б���ֵ��
	 �ڵ���d.func()��ʹ��shared_from_this()����ʱ
	 http://blog.csdn.net/yockie/article/details/40213331
	
	loop_->runAfter(retryDelayMs_ / 1000.0,
	boost::bind(&Connector::startInLoop, shared_from_this()));
	
	*/
	shared_ptr<Connector> pConnector(new Connector(&loop, addr));

	pConnector->setNewConnectionCallback( connectCallback );

	pConnector->start();

	loop.loop();

	 
	 


}