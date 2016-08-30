#ifndef _HUB_CODEC_H
#define _HUB_CODEC_H
#include<msmuduo/net/Buffer.h>
#include<boost/noncopyable.hpp>

#define CURR_NS_BEGIN namespace pubsub {
#define CURR_NS_END }
#define CURR_NS_USING using namespace pubsub

namespace pubsub
{
	enum ParseResult
	{
		kError,
		kSuccess,
		kContinue,
	};

	ParseResult parseMessage(ms::Buffer* buf,
		string* cmd,
		string* topic,
		string* content);

}

#endif