#include"../stdafx.h"
#include"LogFile.h"
#include"FileUtil.h"


#include <assert.h>
#include <stdio.h>
#include <time.h>


NS_USING;

LogFile::LogFile(const string& basename,
	size_t rollSize,
	bool threadSafe,
	int flushInterval,
	int checkEveryN)
	: basename_(basename),
	rollSize_(rollSize),
	flushInterval_(flushInterval),
	checkEveryN_(checkEveryN),
	count_(0),
	mutex_(threadSafe ? new mutex_t : NULL), //�õ���scoped_ptr�����Զ��ͷ��ڴ�
	startOfPeriod_(0),
	lastRoll_(0),
	lastFlush_(0)
{
	assert(basename.find('/') == string::npos);
	rollFile();
}


LogFile::~LogFile()
{
}

void LogFile::append(const char* logline, int len)
{
	if (mutex_)
	{
		lock_guard_t lock(*mutex_);
		append_unlocked(logline, len);
	}
	else
	{
		append_unlocked(logline, len);
	}
}


void LogFile::flush()
{
	if (mutex_)
	{
		lock_guard_t lock(*mutex_);
		file_->flush();
	}
	else
	{
		file_->flush();
	}
}

void LogFile::append_unlocked(const char* logline, int len)
{
	file_->append(logline, len);

	 
	if (file_->writtenBytes() > rollSize_)
	{
		 
		rollFile();
	}
	else
	{
		 
		++count_;
		if (count_ >= checkEveryN_)
		{
			count_ = 0;
			time_t now = ::time(NULL);
			time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
			if (thisPeriod_ != startOfPeriod_) //�µ�һ��
			{
				rollFile();
			}

			else if (now - lastFlush_ > flushInterval_)
			{
				lastFlush_ = now;
				file_->flush();
			}
		}
	}

}

bool LogFile::rollFile()
{
	time_t now = 0;
	string filename = getLogFileName(basename_, &now);
	time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

	if (now > lastRoll_)
	{
		lastRoll_ = now;
		lastFlush_ = now;
		startOfPeriod_ = start;
		file_.reset(new FileUtil::AppendFile(filename));
		return true;
	}
	return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now)
{
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);
	//gmtime_r(now, &tm); // FIXME: localtime_r ?
#ifdef WIN
 	localtime_s(&tm, now);  
#else
	//gmtime_r(&seconds, &tm_time);  
	localtime_s(&tm, now);

#endif
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	filename += timebuf;

	 
	filename += ".log";

	return filename;
}