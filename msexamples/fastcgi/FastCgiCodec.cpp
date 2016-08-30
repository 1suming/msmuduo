#include"stdafx.h"
#include"FastCgiCodec.h"
#include<msmuduo/base/Logging.h>
#include<msmuduo/base/endiantool.h>
 
//��ͷ�Ͱ������һ��Record.

//Fastcgi��һ����������Э�飬������8�ֽڶ���.
struct FastCgiCodec::RecordHeader
{
	uint8_t version;
	uint8_t type;
	uint16_t id; //request id
	uint16_t length; //content length
	uint8_t padding; //��䳤�ȣ�Ϊ��8�ֽڶ���
	uint8_t reserved;

};

const unsigned FastCgiCodec::kRecordHeader = static_cast<unsigned>(sizeof(FastCgiCodec::RecordHeader));//��Ĵ�С�ڱ����ڼ��Ѿ�ȷ��

//fastcgi��Ϣ����
 
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

//role��ʾweb������ϣ��Ӧ�ð��ݵĽ�ɫ��һ����Ϊ��Ӧ����ɫ
enum FcgiRole
{
	// kFcgiInvalid = 0,
	kFcgiResponder = 1,
	kFcgiAuthorizer = 2,
};
//��FCGI_BEGIN_REQUEST�е�flags�������һ��������·�رյ�λ��flags & FCGI_KEEP_CONN�����Ϊ0����Ӧ���ڶԱ���������Ӧ��ر���·�������0��Ӧ���ڶԱ���������Ӧ�󲻻�ر���·��Web������Ϊ��·������Ӧ�ԡ�
enum FcgiConstant
{
	kFcgiKeepConn=1,
};

NS_USING;

//���ȷ��͵���FCGI_BEGIN_REQUEST��Ȼ����FCGI_PARAMS��FCGI_STDIN������ÿ����Ϣͷ�����潫��ϸ˵���������ܹ����ص���󳤶���65535���������������͵���Ϣ��һ��ֻ����һ�Σ��п����������Ͷ�Ρ�
//FastCGI ��Ӧ�崦�����֮�󣬽�����FCGI_STDOUT��FCGI_STDERR��ͬ��Ҳ���ܶ���������͡������FCGI_END_REQUEST��ʾ����Ľ�����
//��Ҫע���һ�㣬FCGI_BEGIN_REQUEST��FCGI_END_REQUEST�ֱ��ʶ������Ŀ�ʼ�ͽ���


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
��˵��Э����FCGI_PARAMS�е�Name - Value Pairs��Ŀ�����ṩӦ�ò�һЩ��Ҫ�ı���������http�е�header��headerName - headerValue����Ȼ����Ϊ���������ϸ�����http://www.fastcgi.com/devkit/doc/fcgi-spec.html#S3.4
����һ�ֶ����ʽ���£�
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

���й涨���ֻ���ֵ�ĳ����������127�ֽڣ���Ҫ��4�ֽڴ洢������
���� 0  1  2  3  4  5  6  7 ............
��ֵ 0F 80 00 00 91 S  C  R IPT_FILENAME/data/www/......

����0��0F��ʮ���Ƶ�15��SCRIPT_FILENAME�ĳ��ȣ���������127����ռһ���ֽ�
����1��80��ʮ���Ƶ�128������127��˵��Ҫռ��4�ֽڣ�80 00 00 91��������Ϊ
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0
������ڶ����أ������λ�ơ���Ȳ������Ų���Ϥ�Ļ�������ϸ�Ľ��ܼ�֮ǰ������
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

//��name,value�ĳ��ȵ�����һ���ֽڻ���4���ֽ�
uint32_t FastCgiCodec::readLen()
{
	if (paramsStream_.readableBytes() >= 1)
	{
		uint8_t byte = paramsStream_.peekInt8();
		if (byte & 0x80) //���ȴ���127,��4�ֽڴ洢
		{
			if (paramsStream_.readableBytes() >= sizeof(uint32_t))
			{
				return paramsStream_.readInt32() & 0x7fffffff;//nameLen��valueLen���޷�����
			}
			else
			{
				return -1;
			}
		}
		else
		{
			//<=127����1�ֽڴ洢
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

	if (header.length >= kRecordHeader) //fcgi_begin_request�ĳ���Ϊ8
	{
		uint16_t role = readInt16(buf->peek() + kRecordHeader);
		uint8_t flags = buf->peek()[kRecordHeader + sizeof(uint16_t)];
		if (role == kFcgiResponder)//�������Ӧ����ɫ
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
		gotRequest_ = true; //����gotRequestΪtrue
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
			static_cast<uint8_t>(-response->readableBytes() & 7),//padding  ,�����ٸ��ֽ�
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