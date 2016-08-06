#include"msmuduo/stdafx.h"
#include"base/AsyncLogging.h"
#include"base/Logging.h"
#include<iostream>
 
using namespace std;

NS_USING;

int kRollSize = 500 * 1000 * 1000; //写入的byte达到多少就roll file

AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);

}
void bench(bool longLog)
{
	Logger::setOutput(asyncOutput);

 
	int cnt = 0;
	const int kBatch = 1000;
	string empty = " ";
	string longStr(3000, 'X');
	longStr += " ";

	for (int t = 0; t < 30; t++)
	{
		Timestamp start = Timestamp::now();
		for (int i = 0; i < kBatch; i++)
		{
			LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
				<< (longLog ? longStr : empty)
				<< cnt;
			++cnt;
		}

		Timestamp end = Timestamp::now();
		printf("%f\n", timeDifference(end, start) * 1000000 / kBatch);

	}

}
int main(int argc,char *argv[])
{
	char name[256];
	strncpy(name, argv[0], 256);

	string progname = FileUtil::getFileNameNotExtFromPath(name);

	AsyncLogging log(progname, kRollSize);

	log.start();

	g_asyncLog = &log;

 
	bool longLog = argc > 1;
	bench(longLog);


}