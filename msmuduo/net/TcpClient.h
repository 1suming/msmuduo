#ifndef _TcpClient_h
#define _TcpClient_h

#include<boost/noncopyable.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/bind.hpp>

#include"msmuduo/base/lock.h"
#include"msmuduo/net/TcpConnection.h"
#include"msmuduo/net/Connector.h"

// This is a public header file, it must only include public header files.

NS_BEGIN 

 
class EventLoop;

typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable
{
public:
	TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg);
	~TcpClient();

	void connect();
	void disconnect();
	void stop();


	TcpConnectionPtr connection() const
	{
		lock_guard_t guard(mutex_);
		return connection_;
	}

	EventLoop* getLoop() const { return loop_; }
	bool retry() const;

	void enableRetry() { retry_ = true; }

	const string& name() const
	{
		return name_;
	}

	//set connection callback
	//Not thread safe
	void setConnectionCallback(const ConnectionCallback& cb)
	{connectionCallback_ = cb;}

	//set message callback
	//Not thread Safe
	void setMessageCallback(const MessageCallback& cb)
	{messageCallback_ = cb;}

	//set Write complete callback
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{writeCompleteCallback_ = cb;}

private:

	//Not Thread Safe,but in loop
	void newConnection(int sockfd);
	//Not thread safe, but in loop
	void removeConnection(const TcpConnectionPtr& conn);

private:
	EventLoop* loop_;
	ConnectorPtr connector_; //avoid  revealing Connector
	const string name_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;

	bool retry_;//atomic 
	bool connect_;//atomic
	//always in loop thread
	int nextConnId_;
	mutable mutex_t mutex_;
	TcpConnectionPtr connection_; //@GuardedBy mutex_
 

};


NS_END
#endif