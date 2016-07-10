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
	acceptChannel_(loop, acceptSocket_.fd()),//构造一个listenfd的channel
	listenning_(false)
	//idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	//  assert(idleFd_ >= 0);
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reuseport);
	acceptSocket_.bindAddress(listenAddr);

	//ReadEventCallback有个参数Timestamp,这里我没点handleRead没有
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
	//用while来包围accept
	//在Socket的accpet方法里面，就已经把connfd设置非阻塞了。
	int connfd = acceptSocket_.accept(&peerAddr);//!Important:connfd为非阻塞
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