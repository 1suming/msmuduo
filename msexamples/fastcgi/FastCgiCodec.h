#ifndef MUDUO_EXAMPLES_FASTCGI_FASTCGI_H
#define MUDUO_EXAMPLES_FASTCGI_FASTCGI_H


#include<msmuduo/net/TcpConnection.h>

#include<map>


// one FastCgiCodec per TcpConnection
// both lighttpd and nginx do not implement multiplexing,
// so there is no concurrent requests of one connection.

class FastCgiCodec :boost::noncopyable
{
public:
	typedef std::map<string, string> ParamMap;

	typedef boost::function<void(const ms::TcpConnectionPtr& conn,
		ParamMap&,
		ms::Buffer*)> Callback;


	explicit FastCgiCodec(const Callback& cb):
		cb_(cb),
		gotRequest_(false),
		keepConn_(false)
	{

	}


	void onMessage(const ms::TcpConnectionPtr& conn, ms::Buffer* buf, ms::Timestamp receiveTime)
	{
		parseRequest(buf);
		if (gotRequest_)
		{
			cb_(conn, params_, &stdin_);
			stdin_.retrieveAll();
			paramsStream_.retrieveAll();

			params_.clear();
			gotRequest_ = false;

			if (!keepConn_)
			{
				conn->shutdown();
			}

		}

	}

	static void respond(ms::Buffer* response);
private:
	static void endStdout(ms::Buffer* buf);
	static void endRequest(ms::Buffer* buf);
	
private:
	struct RecordHeader;
	bool parseRequest(ms::Buffer* buf);

	bool onBeginRequest(const RecordHeader& header, const ms::Buffer* buf);
	void onStdin(const char* content, uint16_t length);
	bool onParams(const char* content, uint16_t length);
	bool parseAllParams();
	uint32_t readLen();

private:

	Callback cb_;
	bool gotRequest_;
	bool keepConn_;
	ms::Buffer stdin_;
	ms::Buffer paramsStream_; //FCGI_PARAMS             =  4, /* [in]  environment variables       */

	ParamMap params_;

	const static unsigned kRecordHeader;


};

#endif