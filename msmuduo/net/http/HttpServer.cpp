#include"msmuduo/stdafx.h"
#include"msmuduo/net/http/HttpServer.h"

#include"msmuduo/base/Logging.h"

#include"msmuduo/net/http/HttpContext.h"
#include"msmuduo/net/http/HttpRequest.h"
#include"msmuduo/net/http/HttpResponse.h"


#include<boost/bind.hpp>

NS_BEGIN
namespace detail
{
	void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
	{
		resp->setStatusCode(HttpResponse::k404NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);

	}
}
NS_END

NS_USING;


HttpServer::HttpServer(EventLoop* loop,
	const InetAddress& listenAddr,
	const string& name,
	TcpServer::Option option)
	: server_(loop, listenAddr, name, option),
	httpCallback_(detail::defaultHttpCallback)
{
	server_.setConnectionCallback(
		boost::bind(&HttpServer::onConnection, this, _1));
	server_.setMessageCallback(
		boost::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
	LOG_WARN << "HttpServer[" << server_.name()
		<< "] starts listenning on " << server_.hostport();
	server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		conn->setContext(HttpContext());
	}
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
	Buffer* buf,
	Timestamp receiveTime)
{
	HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());

	if (!context->parseRequest(buf, receiveTime))
	{
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}

	if (context->gotAll())
	{
		onRequest(conn, context->request());
		context->reset();
	}
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
	// const对象默认调用const成员函数，非const对象默认调用非const成员函数；
	//若const对象想调用非const成员函数，同理const_cast<Student&>(constObj).getAge();(注意：constObj要加括号)
	//: error C2662: “std::string ms::HttpRequest::getHeader(const std::string &)”: 不能将“this”指针从“const ms::HttpRequest”转换为“ms::HttpRequest &”
	//所以要写一个const成员函数
	const string& connection = req.getHeader("Connection");
	bool close = connection == "close" ||
		(req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
	HttpResponse response(close);
	httpCallback_(req, &response);
	Buffer buf;
	response.appendToBuffer(&buf); //把response输出到buffer
	conn->send(&buf);
	LOG_DEBUG<<"send complete";
	if (response.closeConnection())
	{
		LOG_DEBUG << "closeConnection shutdown";
		conn->shutdown();
	}
}
