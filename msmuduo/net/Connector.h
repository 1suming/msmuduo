#ifndef _Connector_h
#define _Connector_h

#include"net/InetAddress.h"
#include<boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include<boost/scoped_ptr.hpp>
#include"base/Logging.h"

NS_BEGIN

class Channel;
class EventLoop;


class Connector :boost::noncopyable,
					public boost::enable_shared_from_this<Connector>
{

public:
	typedef boost::function<void(int sockfd)> NewConnectionCallback;


	Connector(EventLoop* loop, const InetAddress& serverAddr):
		loop_(loop),
		serverAddr_(serverAddr),
		connect_(false),
		state_(kDisconnected),
		retryDelayMs_(kInitRetryDelayMs)
	{
		LOG_DEBUG << "Connector:Constructor[" << this << "]";
	}

	~Connector()
	{
		LOG_DEBUG << "Connector::Destructor[" << this << "]";
		assert(!channel_);
	}




	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{newConnectionCallback_ = cb;}

	void start(); //can be called in any thread
	void restart(); //must be called in loop thread
	void stop();// can be called in any thread


	const InetAddress& serverAddress() const { return serverAddr_; }





private:

	void startInLoop();
	void stopInLoop();
	void connect();
	void connecting(int sockfd);
	void handleWrite();
	void handleError();
	void retry(int sockfd);
	int removeAndResetChannel();
	void resetChannel();





private:
	enum States {
		kDisconnected,
		kConnecting,
		kConnected,

	};

	static const int kMaxRetryDelayMs = 30 * 1000;
	static const int kInitRetryDelayMs = 500;

	void setState(States s) { state_ = s; }



	EventLoop* loop_;
	InetAddress serverAddr_;
	bool connect_;
	States state_; //FIXME:use atomic variable 
	boost::scoped_ptr<Channel> channel_; //!!Important:


	NewConnectionCallback newConnectionCallback_;

	int retryDelayMs_;

};

NS_END
#endif