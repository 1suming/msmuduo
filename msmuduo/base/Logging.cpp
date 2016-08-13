#include"../stdafx.h"
#include"Logging.h"

#ifdef WIN
#include<time.h>
#else
#include <sys/time.h>

#endif

 
NS_BEGIN

threadlocal char t_errnobuf[512];
threadlocal char t_time[32];
threadlocal time_t t_lastSecond;

//全局函数，返回一个enum LogLevel
Logger::LogLevel  initLogLevel()
{
	return Logger::DEBUG; //一般正式环境定义成Logger::INFO
}


Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"OK    ",
	"WARN  ",
	"ERROR ",
	"FATAL ",
};

/** helper class for known string length  at compile time */
class T
{
public:
 
	T(const char* str, unsigned len)
		:str_(str),
		len_(len)
	{
		assert(strlen(str) == len_);
	}

	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}
 


void defaultOutput(const char* msg, int len)
{
	size_t n = fwrite(msg, 1, len, stdout);//第二个参数：size：每个数据类型的大小； 第3个参数:count：数据的个数
	//FIXME check n
	(void)n;
}
void defaultFlush()
{
	fflush(stdout);
}


Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;



NS_END  /************namespace end ******************/


NS_USING;

void Logger::formatTime()
{
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
	int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);

	if (seconds != t_lastSecond) //如果seconds一样不用重新获取t_time,加快速度
	{
		t_lastSecond = seconds;
		struct tm tm_time;
		
		//::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
		

		#ifdef WIN
					//gmtime_s(&tm_time, &seconds);//thread-safe gmtime
					localtime_s(&tm_time, &seconds); //localtime_r得到小时数会多8，因为china是+8区。同样建议使用localtime_r版本。
		#else
			//gmtime_r(&seconds, &tm_time); //linux下gmtime_r为可重入版本
			localtime_s(&tm_time, &seconds); //localtime_r得到小时数会多8，因为china是+8区。同样建议使用localtime_r版本。
		#endif


		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17); (void)len;

	}

	Fmt us(".%06dZ ", microseconds);
	assert(us.length() == 9);
	stream_ << T(t_time, 17) << T(us.data(), 9); //T类定义看本文件



}
Logger::~Logger()
{
	stream_ << " - " << basename_ << ':' << line_ << '\n';//注意是'\n'

	//这里加上色彩
	if (level_ == OK)
		echotool::changeToColor(COLOR_GREEN);
	else if (level_ == ERROR || level_==FATAL)
		echotool::changeToColor(COLOR_RED);
	else if (level_ == WARN)
		echotool::changeToColor(COLOR_YELLOW);


	const LogStream::Buffer& buf(stream().buffer());

	


	g_output(buf.data(), buf.length()); //!析构时，才输出内容
	
	//--------------
	if (level_ == OK || level_ == ERROR || level_ == FATAL || level_ == WARN)
		echotool::changeToOldColor();


	if (level_ == FATAL)//FATAL 终止
	{
		g_flush();
		abort();

	}

}

void Logger::logToStream()
{
	formatTime();
	CurrentThread::tid();
	stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
	stream_ << T(LogLevelName[level_], 6);
}



void Logger::setLogLevel(Logger::LogLevel level)
{
	g_logLevel = level;
}
void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}
