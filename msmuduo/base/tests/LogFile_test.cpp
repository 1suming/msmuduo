#include"../../stdafx.h"
#include"../LogFile.h"
#include"../FileUtil.h"
 #include<iostream>

NS_USING;



boost::scoped_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
	g_logFile->append(msg, len);
}
void flushFunc()
{
	g_logFile->flush();
}

int main(int argc, char* argv[])
{
	char name[256];
	strncpy(name, argv[0], 256);

	string programName = FileUtil::getFileNameNotExtFromPath(name);


	cout << "programName:" << programName << endl;
	g_logFile.reset(new LogFile(programName, 200 * 1000));
	/* scoped_ptr reset 调用的是swap
	 void reset(T * p = 0) // never throws
	 {
	 BOOST_ASSERT( p == 0 || p != px ); // catch self-reset errors
	 this_type(p).swap(*this);
	 }
	 */
	sleep(1000 * 1);

	char buf[80];
	 

	string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < 10000; i++)
	{
		cout <<i<<" hello" << endl;
		sprintf(buf, "%d", i);
		string inputStr(buf);
		inputStr = inputStr + line;
		outputFunc(inputStr.c_str(), line.length());



	}

	flushFunc();
}

