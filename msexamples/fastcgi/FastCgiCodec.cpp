#include"stdafx.h"
#include"FastCgiCodec.h"
#include<msmuduo/base/Logging.h>
#include<msmuduo/base/endiantool.h>
 
//包头和包体组成一个Record.

//Fastcgi是一个基于流的协议，并且是8字节对齐.
struct FastCgiCodec::RecordHeader
{
	uint8_t version;
	uint8_t type;
	uint16_t id; //request id
	uint16_t length; //content length
	uint8_t padding; //填充长度，为了8字节对齐
	uint8_t reserved;

};

const unsigned FastCgiCodec::kRecordHeader = static_cast<unsigned>(sizeof(FastCgiCodec::RecordHeader));//类的大小在编译期间已经确定

//fastcgi消息类型
 
typedef enum _fcgi_request_type {
	FCGI_BEGIN_REQUEST      =  1, /* [in]                              */
	FCGI_ABORT_REQUEST      =  2, /* [in]  (not supported)             */
	FCGI_END_REQUEST        =  3, /* [out]                             */
	FCGI_PARAMS             =  4, /* [in]  environment variables       */
	FCGI_STDIN              =  5, /* [in]  post data                   */
	FCGI_STDOUT             =  6, /* [out] response                    */
	FCGI_STDERR             =  7, /* [out] errors                      */
	FCGI_DATA               =  8, /* [in]  filter data (not supported) */
	    FCGI_GET_VALUES         =  9, /* [in]                              */
	    FCGI_GET_VALUES_RESULT  = 10  /* [out]                             */
} fcgi_request_type;
 
enum FcgiType
{
	kFcgiInvalid=0,
	kFcgiBeginRequest=1,
	kFcgiAbortRequest=2,
	kFcgiEndRequest=3,
	kFcgiParams=4,
	kFcgiStdin=5,

	kFcgiStdout=6,
	kFcgiStderr=7,
	kFcgiData=8,
	kFcgiGetValues=9,
	kFcgiGetValuesResult=10,

};

//role表示web服务器希望应用扮演的角色，一般作为响应器角色
enum FcgiRole
{
	// kFcgiInvalid = 0,
	kFcgiResponder = 1,
	kFcgiAuthorizer = 2,
};
//而FCGI_BEGIN_REQUEST中的flags组件包含一个控制线路关闭的位：flags & FCGI_KEEP_CONN：如果为0，则应用在对本次请求响应后关闭线路。如果非0，应用在对本次请求响应后不会关闭线路；Web服务器为线路保持响应性。
enum FcgiConstant
{
	kFcgiKeepConn=1,
};

NS_USING;

//最先发送的是FCGI_BEGIN_REQUEST，然后是FCGI_PARAMS和FCGI_STDIN，由于每个消息头（下面将详细说明）里面能够承载的最大长度是65535，所以这两种类型的消息不一定只发送一次，有可能连续发送多次。
//FastCGI 响应体处理完毕之后，将发送FCGI_STDOUT、FCGI_STDERR，同理也可能多次连续发送。最后以FCGI_END_REQUEST表示请求的结束。
//需要注意的一点，FCGI_BEGIN_REQUEST和FCGI_END_REQUEST分别标识着请求的开始和结束


bool FastCgiCodec::parseRequest(Buffer* buf)
{
	while (buf->readableBytes() >= kRecordHeader)
	{
		RecordHeader header; 
		memcpy(&header, buf->peek(), kRecordHeader);
		header.id = endiantool::networkToHost16(header.id);
		header.length = endiantool::hostToNetwork16(header.length);

		size_t total = kRecordHeader + header.length + header.padding;

		if (buf->readableBytes() >= total)
		{
			LOG_WARN << "header type:" << header.type;
			switch (header.type)
			{
				
			case kFcgiBeginRequest:
				onBeginRequest(header, buf);
				//FIXME:check
				break;
			case kFcgiParams: //FCGI_PARAMS             =  4, /* [in]  environment variables       */
				onParams(buf->peek() + kRecordHeader, header.length);
				//FIXME:check
				break;
			case kFcgiStdin:
				onStdin(buf->peek() + kRecordHeader, header.length);
				break;
			case kFcgiData:
				// FIXME:
				break;
			case kFcgiGetValues:
				// FIXME:
				break;
			default:
				// FIXME:
				break;

			}

			buf->retrieve(total);
		}
		else
		{
			break;
		}


	}
	return true;
}

//FCGI_PARAMS             =  4, /* [in]  environment variables       */
bool FastCgiCodec::onParams(const char* content, uint16_t length)
{
	if (length > 0)
	{
		paramsStream_.append(content, length);
	}
	else if (!parseAllParams())
	{
		LOG_ERROR << "parseAllParams() failed";
		return false;
	}
	return true;
}

/*
再说下协议中FCGI_PARAMS中的Name - Value Pairs，目的是提供应用层一些必要的变量（类似http中的header：headerName - headerValue，当然可以为多个），详细定义见http://www.fastcgi.com/devkit/doc/fcgi-spec.html#S3.4
其中一种定义格式如下：
typedef struct {
	unsigned char nameLengthB0; // nameLengthB0 >> 7 == 0  
	unsigned char valueLengthB3; // valueLengthB3 >> 7 == 1  
	unsigned char valueLengthB2;
	unsigned char valueLengthB1;
	unsigned char valueLengthB0;
	unsigned char nameData[nameLength];
	unsigned char valueData[valueLength
		((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
} FCGI_NameValuePair14;

其中规定名字或者值的长度如果大于127字节，则要以4字节存储，如下
序列 0  1  2  3  4  5  6  7 ............
数值 0F 80 00 00 91 S  C  R IPT_FILENAME/data/www/......

序列0的0F即十进制的15（SCRIPT_FILENAME的长度），不大于127所以占一个字节
序列1的80即十进制的128，大于127，说明要占用4字节（80 00 00 91），长度为
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0
算算等于多少呢？如果对位移、与等操作符号不熟悉的话，更详细的介绍见之前的文章
*/
bool FastCgiCodec::parseAllParams()
{
	while (paramsStream_.readableBytes() > 0)
	{
		uint32_t nameLen = readLen();
		if (nameLen == static_cast<uint32_t>(-1))
		{
			return false;
		}
		uint32_t valueLen = readLen();
		if (valueLen == static_cast<uint32_t>(-1))
		{
			return false;
		}

		if (paramsStream_.readableBytes() >= nameLen + valueLen)
		{
			string name = paramsStream_.retrieveAsString(nameLen);
			params_[name] = paramsStream_.retrieveAsString(valueLen);
		}
		else
		{
			return false;
		}
	}
	return true;
}

//看name,value的长度到底是一个字节还是4个字节
uint32_t FastCgiCodec::readLen()
{
	if (paramsStream_.readableBytes() >= 1)
	{
		uint8_t byte = paramsStream_.peekInt8();
		if (byte & 0x80) //长度大于127,用4字节存储
		{
			if (paramsStream_.readableBytes() >= sizeof(uint32_t))
			{
				return paramsStream_.readInt32() & 0x7fffffff;//nameLen和valueLen是无符号数
			}
			else
			{
				return -1;
			}
		}
		else
		{
			//<=127，用1字节存储
			return paramsStream_.readInt8();
		}
	}
	else
	{
		return -1;
	}
}

uint16_t readInt16(const void* p)
{
	uint16_t be16 = 0;
	::memcpy(&be16, p, sizeof be16);
	return endiantool::hostToNetwork16(be16);

}
/*
typedef struct _fcgi_begin_request {
unsigned char roleB1;
unsigned char roleB0;
unsigned char flags;
unsigned char reserved[5];
} fcgi_begin_request;

*/
bool FastCgiCodec::onBeginRequest(const RecordHeader& header, const ms::Buffer* buf)
{
	assert(buf->readableBytes() >= header.length);
	assert(header.type == kFcgiBeginRequest);

	if (header.length >= kRecordHeader) //fcgi_begin_request的长度为8
	{
		uint16_t role = readInt16(buf->peek() + kRecordHeader);
		uint8_t flags = buf->peek()[kRecordHeader + sizeof(uint16_t)];
		if (role == kFcgiResponder)//如果是响应器角色
		{
			keepConn_ = (flags == kFcgiKeepConn);
			return true;
		}
	}
	return false;
}

void FastCgiCodec::onStdin(const char* content, uint16_t length)
{
	if (length > 0)
	{
		stdin_.append(content, length);

	}
	else
	{
		gotRequest_ = true; //设置gotRequest为true
	}
}

void FastCgiCodec::respond(Buffer* response)
{
	if (response->readableBytes() < 65536
		&& response->prependableBytes() >= kRecordHeader)
	{
		RecordHeader header =
		{
			1,
			kFcgiStdout,
			endiantool::hostToNetwork16(1), //request id
			endiantool::hostToNetwork16(static_cast<uint16_t>(response->readableBytes())),//conteng length 
			static_cast<uint8_t>(-response->readableBytes() & 7),//padding  ,填充多少个字节
			0,

		};
		response->prepend(&header, kRecordHeader);

		response->append("\0\0\0\0\0\0\0\0", header.padding);

	}
	else
	{
		//FIXME:
	}

	endStdout(response);
	endRequest(response);

}
void FastCgiCodec::endStdout(Buffer* buf)
{
	RecordHeader header =
	{
		1,
		kFcgiStdout,
		endiantool::hostToNetwork16(1),
		0,
		0,
		0,
	};
	buf->append(&header, kRecordHeader);

}

/*

1. typedef struct _fcgi_end_request {
2.     unsigned char appStatusB3;
3.     unsigned char appStatusB2;
4.     unsigned char appStatusB1;
5.     unsigned char appStatusB0;
6.     unsigned char protocolStatus;
7.     unsigned char reserved[3];
8. } fcgi_end_request;


*/
void FastCgiCodec::endRequest(Buffer* buf)
{
	RecordHeader header =
	{
		1,
		kFcgiEndRequest,
		endiantool::hostToNetwork16(1),
		endiantool::hostToNetwork16(kRecordHeader),
		0,
		0,
	};
	buf->appendInt32(0);
	buf->appendInt32(0);

}