#ifndef _Acceptor_h
#define _Acceptor_h

#include<boost/function.hpp>
#include<boost/noncopyable.hpp>
 
#include"net/Channel.h"
#include"net/Socket.h"

 NS_BEGIN

class EventLoop;
class InetAddress;

/*
Acceptor of incoming TCP Connection
*/
class Acceptor : boost::noncopyable
{
public:
	typedef boost::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport=false);
	~Acceptor();


	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{
		newConnectionCallback_ = cb;
	}

	bool listenning()const { return listenning_; }
	void listen();



private:

	void handleRead();


	EventLoop* loop_;
	Socket acceptSocket_; //·Ç×èÈûsocket
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;

	bool listenning_;
	int idleFd_;



};



NS_END
#endif