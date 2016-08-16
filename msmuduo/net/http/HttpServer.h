#ifndef _HttpServer_h
#define _HttpServer_h

#include"msmuduo/net/TcpServer.h"
#include<boost/noncopyable.hpp>

NS_BEGIN

class HttpRequest;
class HttpResponse;

/*
As simple embeddable Htpp server designed  for report status of a program.
It is not a fully Http 1.1 compliant server,but provides minimum features 
that can communicate with HttpClient and web browser.
It is Synchronous ,just like java Servlet.
*/
class HttpServer : boost::noncopyable
{
public:
	typedef boost::function<void(const HttpRequest&,
		HttpResponse*)> HttpCallback;

	HttpServer(EventLoop* loop,
		const InetAddress& listenAddr,
		const string& name,
		TcpServer::Option option = TcpServer::kNoReusePort);

	~HttpServer();  // force out-line dtor, for scoped_ptr members.

	EventLoop* getLoop() const { return server_.getLoop(); }

	/// Not thread safe, callback be registered before calling start().
	void setHttpCallback(const HttpCallback& cb)
	{
		httpCallback_ = cb;
	}

	void setThreadNum(int numThreads)
	{
		server_.setThreadNum(numThreads);
	}

	void start();

private:
	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn,
		Buffer* buf,
		Timestamp receiveTime);
	void onRequest(const TcpConnectionPtr&, const HttpRequest&);

	TcpServer server_;
	HttpCallback httpCallback_;
};



NS_END
#endif