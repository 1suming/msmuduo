#ifndef _HttpRequest_h
#define _HttpRequest_h

#include"msmuduo/stdafx.h"

#include"msmuduo/base/Copyable.h"
#include"msmuduo/base/Timestamp.h"
 
#include<assert.h>
#include<stdio.h>


NS_BEGIN

class HttpRequest : public copyable
{
public:
	enum Method
	{
		kInvalid,kGet,kPost,kHead,kPut,kDelete 
	};

	enum Version
	{
		kUnkown,kHttp10,kHttp11
	};

	HttpRequest():
		method_(kInvalid),
		version_(kUnkown)
	{

	}
	void setVersion(Version v)
	{
		version_ = v;
	}
	Version getVersion() const { return version_; }

	bool setMethod(const char* start, const char* end)
	{
		assert(method_ == kInvalid);
		string m(start, end);
		if (m == "GET")
		{
			method_ = kGet;
		}
		else if (m == "POST")
		{
			method_ = kPost;
		}
		else if (m == "HEAD")
		{
			method_ = kHead;
		}
		else if (m == "PUT")
		{
			method_ = kPut;
		}
		else if (m == "DELETE")
		{
			method_ = kDelete;
		}
		else
		{
			method_ = kInvalid;
		}

		return method_ != kInvalid;
	}

	Method method() const { return method_; }

	const char* methodString() const
	{
		const char* result = "UNKNOWN";
		switch (method_)
		{
		case kGet:
			result = "GET";
			break;
		case kPost:
			result = "POST";
			break;
		case kHead:
			result = "HEAD";
			break;
		case kPut:
			result = "PUT";
			break;
		case kDelete:
			result = "DELETE";
			break;
		default:
			break;
		}
		return result;
	}

	void setPath(const char* start, const char* end)
	{
		path_.assign(start, end);
	}
	const string& path() const
	{
		return path_;
	}
	void setQuery(const char* start, const char* end)
	{
		query_.assign(start, end);
	}

	const string& query() const
	{
		return query_;
	}

	void setReceiveTime(Timestamp t)
	{
		receiveTime_ = t;
	}
	Timestamp receiveTime() const
	{
		return receiveTime_;
	}
	//colon:第一个冒号的位置
	void addHeader(const char* start, const char* colon, const char* end)
	{
		string field(start, colon); // string  (InputIterator first, InputIterator last);
		++colon;
		//key: value 冒号后面有一个空格
		while (colon < end && isspace(*colon))
		{
			++colon;
		}
		string value(colon, end);
		while (!value.empty() && isspace(value[value.size() - 1]))//为了去除value后面的空格
		{
			value.resize(value.size() - 1);
		}
		headers_[field] = value;

	}

	string getHeader(const string& field) const
	{
		string result;
		std::map<string, string>::const_iterator it = headers_.find(field);
		if (it != headers_.end())
		{
			result = it->second;
		}
		return result;
	}
 
	const std::map<string, string>& headers() const
	{
		return headers_;
	}

	void swap(HttpRequest& that)
	{
		std::swap(method_, that.method_);
		path_.swap(that.path_);
		query_.swap(that.query_);

		receiveTime_.swap(that.receiveTime_);
		headers_.swap(that.headers_);

	}

private:
	Method method_;
	Version version_;
	string path_;
	string query_;
	Timestamp receiveTime_;
	std::map<string, string> headers_;
};



NS_END
#endif