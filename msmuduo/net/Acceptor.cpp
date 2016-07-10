#include"stdafx.h"
#include"Acceptor.h"
#include"EventLoop.h"
#include"InetAddress.h"

#include"base/Logging.h"
#include"base/sockettool.h"

#include<boost/bind.hpp>


NS_USING;



Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport):
	loop_(loop),
	acceptSocket_(sockettool::createNonblockingOrDie()),
	acceptChannel_(loop, acceptSocket_.fd()),//����һ��listenfd��channel
	listenning_(false)
	//idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	//  assert(idleFd_ >= 0);
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reuseport);
	acceptSocket_.bindAddress(listenAddr);

	//ReadEventCallback�и�����Timestamp,������û��handleReadû��
	acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead, this));


}


Acceptor::~Acceptor()
{
	acceptChannel_.disableAll();
	acceptChannel_.remove();

	//::close(idleFd_);

}

void Acceptor::listen()
{
	loop_->assertInLoopThread();
	listenning_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();//!!!Important


}


void Acceptor::handleRead()
{
	loop_->assertInLoopThread();
	InetAddress peerAddr;

	//FIXME loop until no more
	//��while����Χaccept
	//��Socket��accpet�������棬���Ѿ���connfd���÷������ˡ�
	int connfd = acceptSocket_.accept(&peerAddr);//!Important:connfdΪ������
	if (connfd >= 0)
	{
		// string hostport = peerAddr.toIpPort();
		// LOG_TRACE << "Accepts of " << hostport;
		if (newConnectionCallback_)
		{
			newConnectionCallback_(connfd, peerAddr);
		}
		else
		{
			sockettool::close(connfd);
		}

	}
	else
	{
		LOG_ERROR << "in Acceptor::handleRead";
		// Read the section named "The special problem of
		// accept()ing when you can't" in libev's doc.
		// By Marc Lehmann, author of livev.
		if (errno == EMFILE)
		{
			/*
			::close(idleFd_);
			idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			*/

		}
	}


}