#pragma once

#include"msmuduo/base/Logging.h"
#include"msmuduo/net/Buffer.h"
#include"msmuduo/net/TcpConnection.h"

#include<boost/bind.hpp>
#include<boost/function.hpp>

#include<string>

NS_USING;

class LengthHeaderCodec : noncopyable {
public:
	//const TcpConnectionPtr& conn,Buffer* buffer,Timestamp receiveTime
	typedef boost::function<void(const TcpConnectionPtr&,
		const std::string& message,
		Timestamp)> StringMessageCallback;

	//explicit关键字只能用于修饰只有一个参数的类构造函数。作用是表明该构造函数是显示的，非隐士的
	explicit LengthHeaderCodec(const StringMessageCallback& cb) :
		messageCallback_(cb) {

	}
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
	{
		while (buf->readableBytes() >= kHeaderLen) //kHeaderLen==4
		{
			//FIXME:use buffer:peekInt32
			const void* data = buf->peek();
			int32_t be32 = *static_cast<const int32_t*>(data);//sigbus
			const int32_t len = endiantool::networkToHost32(be32);
			if (len > 65536 || len < 0)
			{
				LOG_ERROR << "Invalid Lenght:" << len;
				conn->shutdown();//FIXME disable reading
				break;
			}
			else if (buf->readableBytes() >= len + kHeaderLen)
			{
				buf->retrieve(kHeaderLen);
				std::string message(buf->peek(), len);
				messageCallback_(conn, message, receiveTime);
				buf->retrieve(len);
			}
			else
			{
				break;
			}

		}
	}
	//FIXME:TcpConnectionPtr
	void send(TcpConnection* conn, const StringPiece& message)
	{
		Buffer buf;
		buf.append(message.data(), message.size());
		int32_t len = static_cast<int32_t>(message.size());
		int32_t be32 = endiantool::hostToNetwork32(len);
		buf.prepend(&be32, sizeof(be32));
		conn->send(&buf);

	}

private:
	StringMessageCallback messageCallback_;
	const static size_t kHeaderLen = sizeof(int32_t);

};
