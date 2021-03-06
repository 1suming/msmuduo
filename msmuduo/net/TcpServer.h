#ifndef _TcpServer_h
#define _TcpServer_h

#include"msmuduo/net/TcpConnection.h"
#include"msmuduo/net/Callbacks.h"
#include"msmuduo/base/Atomic.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
 


NS_BEGIN


class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class InetAddress;


/*
Tcp Server, supports single-threaded and thread-pool models
This is an interface class, so don't expose too much details

*/

class TcpServer : boost::noncopyable
{
public:
	typedef boost::function<void(EventLoop*)>  ThreadInitCallback;

	enum Option
	{
		kNoReusePort,
		kReusePort,
	};

	//TcpServer(EventLoop* loop, const InetAddress& listenAddr);
	TcpServer(EventLoop* loop,
		const InetAddress& listenAddr,
		const string& nameArg,
		Option option = kNoReusePort);
	~TcpServer();  // force out-line dtor, for scoped_ptr members.


	const string& hostport() const { return hostport_; }
	const string& name() const { return name_; }
	EventLoop* getLoop() const { return loop_; }



	/// Set the number of threads for handling input.
	///
	/// Always accepts new connection in loop's thread.
	/// Must be called before @c start
	/// @param numThreads
	/// - 0 means all I/O in loop's thread, no thread will created.
	///   this is the default value.
	/// - 1 means all I/O in another thread.
	/// - N means a thread pool with N threads, new connections
	///   are assigned on a round-robin basis.
	void setThreadNum(int numThreads);
	void setThreadInitCallback(const ThreadInitCallback& cb)
	{
		threadInitCallback_ = cb;
	}
	/// /// valid after calling start()

	/*
	boost::shared_ptr<EventLoopThreadPool> threadPool()
	{
		return threadPool_;
	}
	*/
	/// Starts the server if it's not listenning.
	///
	/// It's harmless to call it multiple times.
	/// Thread safe!!!
	void start();

	//Not thread safe
	void setConnectionCallback(const ConnectionCallback& cb)
	{connectionCallback_ = cb;}

	//Not thread safe
	void setMessageCallback(const MessageCallback& cb)
	{messageCallback_ = cb;}

 	/// Not thread safe.
	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{writeCompleteCallback_ = cb;}




private:
	//Not thread safe, but in loop
	void newConnection(int sockfd, const InetAddress& peerAddr);
	//thread safe
	void removeConnection(const TcpConnectionPtr& conn);
	//Not thread safe, but in loop
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<string, TcpConnectionPtr> ConnectionMap;


	EventLoop* loop_; //acceptor loop
	const string hostport_;
	const string name_;
	boost::scoped_ptr<Acceptor> acceptor_;// avoid revealing Acceptor
	
	boost::shared_ptr<EventLoopThreadPool> threadPool_;

	ConnectionCallback connectionCallback_; //callbacks.h���ж���
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	ThreadInitCallback threadInitCallback_;

	AtomicInt32 started_;

	//always in loop thread
	int nextConnId_;
	ConnectionMap  connections_;


};


NS_END
#endif