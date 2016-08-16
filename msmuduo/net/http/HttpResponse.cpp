#include"msmuduo/stdafx.h"
#include"msmuduo/net/http/HttpResponse.h"

#include"msmuduo/net/Buffer.h"

NS_USING;

void HttpResponse::appendToBuffer(Buffer* output) const
{
	//status line+ response header+ response body
	//��HTTP/1.1 200 OK
	char buf[32];
	snprintf(buf, sizeof buf, "HTTP/1.1 %d", statusCode_);
	output->append(buf);
	output->append(statusMessage_);
	output->append("\r\n");

	if (closeConnection_)
	{
		output->append("Connection: close\r\n");
	}
	else
	{
		//vs��֧��%zd,
		snprintf(buf, sizeof buf, "Content-Length: %d\r\n", body_.size()); //%zd ������ת��˵����һ��ʹ�ã���ʾһ��size_tֵ(sizeof���ص�����) (C99)��


		output->append(buf);
		output->append("Connection: Keep-Alive\r\n");
	}

	for (std::map<string, string>::const_iterator it = headers_.begin();
		it != headers_.end(); it++)
	{
		output->append(it->first);
		output->append(": ");
		output->append(it->second);
		output->append("\r\n");


	}

	output->append("\r\n");
	output->append(body_);


}