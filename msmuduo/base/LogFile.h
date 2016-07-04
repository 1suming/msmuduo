#ifndef  _LogFile_H
#define  _LogFile_H


#include"lock.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>


NS_BEGIN
namespace FileUtil
{
	class AppendFile; 

}

class LogFile : boost::noncopyable
{
public:

public:
	LogFile(const string& basename,
		size_t rollSize,
		bool threadSafe = true,
		int flushInterval = 3,
		int checkEveryN = 1024);
	~LogFile();

	void append(const char* logline, int len);
	void flush();
	bool rollFile();

private:
	void append_unlocked(const char* logline, int len);

	static string getLogFileName(const string& basename, time_t* now);

	const string basename_;
	const size_t rollSize_;
	const int flushInterval_; //��secondsΪ��λ
	const int checkEveryN_;

	int count_;

	boost::scoped_ptr<mutex_t> mutex_;
	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;
	boost::scoped_ptr<FileUtil::AppendFile> file_;

	const static int kRollPerSeconds_ = 60 * 60 * 24;

};


NS_END


#endif

