#include"stdafx.h"

#include"msmuduo/base/Logging.h"
#include"msmuduo/net/EventLoop.h"
#include"msmuduo/net/TcpServer.h"


NS_USING;

const char* g_file = NULL;

//// FIXME: use FileUtil::readFile()

string readFile(const char*filename)
{
	string content;
	FILE* fp = ::fopen(filename, "rb");
	if (fp)
	{
		//inefficient
		const int kBufSize = 1024;//��Ҫ��1024*1024����debug��ʱ�����stack overflow
		char iobuf[kBufSize];
		//::setbuf(fp, iobuf); //���庯����void setbuffer(FILE * stream, char * buf, size_t size);
		//void setbuf(FILE *stream,char *buf); 


		char buf[kBufSize];
		size_t nread = 0;
		while ((nread = ::fread(buf, 1, sizeof buf, fp)) > 0)//size_t fread ( void *buffer, size_t size, size_t count, FILE *stream) ;
		{
			content.append(buf, nread); //string &append(const char *s,int n);//��c�����ַ���s��ǰn���ַ����ӵ���ǰ�ַ�����β

		}

		::fclose(fp);

	}
	return content;
}
 
 
void onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
	LOG_INFO << "HighWaterMark " << len;
}
void onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
	if (conn->connected())
	{
		LOG_INFO << "FileServer - Sending file " << g_file<< " to " << conn->peerAddress().toIpPort();
		conn->setHighWaterMarkCallback(onHighWaterMark, 64 * 1024);
		string fileContent = readFile(g_file);
		printf("%s\n", fileContent.c_str());
		conn->send(fileContent);
		conn->shutdown();//!Important:����send�����̹ر���û��Σ�գ���shutdown��ʱ�����ж�if (!channel_->isWriting()) �����û�в�shutdown  WRITE
		LOG_INFO << "FileServer - done";
	}
}

 

int main(int argc, char * argv[])
{
	 
	Logger::setLogLevel(Logger::TRACE);
	const char* file = "F:/vsproject/msmuduo/Debug/a.txt";
	if (argc > 1)
	{
		file = argv[1];
	}
	if (1) //argc > 1)
	{
		g_file = file;
		string fileContent = readFile(g_file);
		printf("%s\n", fileContent.c_str());


		EventLoop loop;
		InetAddress listenAddr(9981);
		TcpServer server(&loop, listenAddr, "fileserver");

		server.setConnectionCallback(onConnection);
		server.start();
		loop.loop();

	}
	else
	{
		fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);

	}
}