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
	// const����Ĭ�ϵ���const��Ա��������const����Ĭ�ϵ��÷�const��Ա������
	//��const��������÷�const��Ա������ͬ��const_cast<Student&>(constObj).getAge();(ע�⣺constObjҪ������)
	//: error C2662: ��std::string ms::HttpRequest::getHeader(const std::string &)��: ���ܽ���this��ָ��ӡ�const ms::HttpRequest��ת��Ϊ��ms::HttpRequest &��
	//����Ҫдһ��const��Ա����
	const string& connection = req.getHeader("Connection");
	bool close = connection == "close" ||
		(req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
	HttpResponse response(close);
	httpCallback_(req, &response);
	Buffer buf;
	response.appendToBuffer(&buf); //��response�����buffer
	conn->send(&buf);
	LOG_DEBUG<<"send complete";
	if (response.closeConnection())
	{
		LOG_DEBUG << "closeConnection shutdown";
		conn->shutdown();
	}
}
