#include"stdafx.h"
#include"FastCgiCodec.h"

#include<msmuduo/base/Logging.h>
#include<msmuduo/net/EventLoop.h>
#include<msmuduo/net/TcpServer.h>

#include<msmuduo/util/hexdump.h>

#include<boost/bind.hpp>

NS_USING;

const string kPath = "/sudoku/";


void onRequest(const TcpConnectionPtr& conn,
	FastCgiCodec::ParamMap& params,
	Buffer* in)//in±Ì æfcgi std in
{
	string uri = params["REQUEST_URI"];
	LOG_OK << conn->name() << ": " << uri;

	for (FastCgiCodec::ParamMap::const_iterator it = params.begin();
		it != params.end(); it++)
	{
		LOG_DEBUG << it->first << " = " << it->second;

	}
	if (in->readableBytes() > 0)
	{
		LOG_DEBUG << "stdin " << in->retrieveAllAsString();
	}
	Buffer response;
	response.append("Context-Type: text/plain\r\n\r\n");
	/*
	if (uri.size() == kCells + kPath.size() && uri.find(kPath) == 0)
	{
		response.append(solveSudoku(uri.substr(kPath.size())));
	}
	else
	{
		// FIXME: set http status code 400
		response.append("bad request");
	}
	*/
	response.append("hello world");

	FastCgiCodec::respond(&response);

	hexdump::hexdump(response.peek(), response.readableBytes());

	conn->send(&response);





}

void onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		typedef boost::shared_ptr<FastCgiCodec> CodecPtr;
		CodecPtr codec(new FastCgiCodec(onRequest));


		conn->setContext(codec);

		conn->setMessageCallback(
			boost::bind(&FastCgiCodec::onMessage, codec, _1, _2, _3));

		conn->setTcpNoDelay(true);

	}
}
int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::TRACE);
	int port = 19981;
	int threads = 0;
	if (argc > 1)
		port = atoi(argv[1]);
	
	if (argc > 2)
	{
		threads = atoi(argv[2]);
	}
	InetAddress addr(static_cast<uint16_t>(port));
	LOG_INFO << "Sudoku FastCGI listens on " << addr.toIpPort()
		<< " threads " << threads;

	EventLoop loop;
	TcpServer server(&loop, addr, "FastCgiTest");

	server.setConnectionCallback(onConnection);
	server.setThreadNum(threads);

 
	server.start();
	loop.loop();


}