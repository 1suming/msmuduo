#include"msmuduo/stdafx.h"
#include"net/Connector.h"
#include"net/EventLoop.h"
#include"net/Acceptor.h"

#include"net/InetAddress.h"
#include"base/sockettool.h"
#include"net/TcpServer.h"
#include"net/TcpClient.h"
#include"base/Logging.h"
#include"net/EventLoopThread.h"


#include<stdio.h>

NS_USING;

void runInThread()
{
	printf("runInThread():  tid = %d\n",CurrentThread::tid());
}

int main(int argc, char* argv[])
{
	printf("main:tid:%d\n", CurrentThread::tid());

	EventLoopThread loopThread;
	EventLoop* loop = loopThread.startLoop();


	loop->runInLoop(runInThread);

	sleep(1);

	loop->runAfter(2, runInThread);

	loop->quit();

	printf("exit main\n");
	
}