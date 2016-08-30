/*
This is an internal header class,you should'nt include this.
http协议解析类
*/

#ifndef _HttpContext_h
#define _HttpContext_h

#include"msmuduo/base/copyable.h"
#include"msmuduo/net/http/HttpRequest.h"


NS_BEGIN

class Buffer;

class HttpContext : public copyable
{
public:
	enum HttpRequestParseState
	{
		kExpectRequestLine,
		kExpectHeaders,
		kExpectBody,
		kGotAll
	};
	HttpContext() :
		state_(kExpectRequestLine)
	{

	}

	// default copy-ctor, dtor and assignment are fine

	//return false if any error
	bool parseRequest(Buffer* buf, Timestamp receiveTime);

	bool gotAll() const
	{
		return state_ == kGotAll;
	}
	void reset()
	{
		state_ = kExpectRequestLine;
		HttpRequest dummy;
		request_.swap(dummy);
	}


	const HttpRequest& request() const
	{
		return request_;
	}

	HttpRequest& request()
	{
		return request_;
	}




private:

	bool processRequestLine(const char* beign, const char* end);

	HttpRequestParseState state_;
	HttpRequest request_;

};

NS_END

#endif