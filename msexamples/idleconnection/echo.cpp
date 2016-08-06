#include"stdafx.h"
#include"echo.h"


#include"msmuduo/base/Logging.h"
#include"msmuduo/net/EventLoop.h"

#include <boost/bind.hpp>

#include <assert.h>
#include <stdio.h>


//shared_ptr/weak_ptr/time wheel 的一个精妙的组合。
//顺便回答楼主的两个问题:
//（1）TcpConnection 和 Entry 之间应该会出现circular reference的现象，结果是谁都死不掉。

EchoServer::EchoServer(EventLoop* loop,
	const InetAddress& listenAddr,
	int idleSeconds)
	: server_(loop, listenAddr, "EchoServer"),
	connectionBuckets_(idleSeconds)
{
	server_.setConnectionCallback(
		boost::bind(&EchoServer::onConnection, this, _1));
	server_.setMessageCallback(
		boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
	loop->runEvery(1.0, boost::bind(&EchoServer::onTimer, this));
	connectionBuckets_.resize(idleSeconds);
	dumpConnectionBuckets();
}

void EchoServer::start()
{
	server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

	if (conn->connected())
	{
		EntryPtr entry(new Entry(conn));
		connectionBuckets_.back().insert(entry);
		dumpConnectionBuckets();
		WeakEntryPtr weakEntry(entry);
		conn->setContext(weakEntry);
	}
	else
	{
		assert(!conn->getContext().empty());
		WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
		LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
	}
}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
	Buffer* buf,
	Timestamp time)
{
	string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size()
		<< " bytes at " << time.toString();
	conn->send(msg);

	assert(!conn->getContext().empty());
	WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
	EntryPtr entry(weakEntry.lock());
	if (entry)
	{
		connectionBuckets_.back().insert(entry);
		dumpConnectionBuckets();
	}
}

void EchoServer::onTimer()
{
	connectionBuckets_.push_back(Bucket());
	dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets() const
{
	LOG_INFO << "size = " << connectionBuckets_.size();
	int idx = 0;
	for (WeakConnectionList::const_iterator bucketI = connectionBuckets_.begin();
		bucketI != connectionBuckets_.end();
		++bucketI, ++idx)
	{
		const Bucket& bucket = *bucketI;
		printf("[%d] len = %d : ", idx, bucket.size());
		for (Bucket::const_iterator it = bucket.begin();
			it != bucket.end();
			++it)
		{
			bool connectionDead = (*it)->weakConn_.expired();
			printf("%p(%ld)%s, ", get_pointer(*it), it->use_count(),
				connectionDead ? " DEAD" : "");
		}
		puts("");
	}
}