#ifndef _Logging_h
#define _Logging_h

#include"Timestamp.h"
#include"LogStream.h"
#include"Thread.h"
#include"FileUtil.h"

#include"echotool.h"

NS_BEGIN

#undef ERROR //ewindows��ERROR��ĳ���ļ����к궨��

class T;
class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,

		INFO,
		OK,//��ʾΪ��ɫ

		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

	Logger(string filename, int line, LogLevel level):
		//basename_(filename),
		line_(line),
		level_(level),

		time_(Timestamp::now()) //����ʱ��ֵtime_
	{
		basename_ = FileUtil::getFileNameFromPath(filename);
		logToStream();

	}
	Logger(string filename, int line, LogLevel level,const char* func): //�����filename��__LINE__ 
		//basename_(filename),
		line_(line),
		level_(level),

		time_(Timestamp::now())
	{
		basename_ = FileUtil::getFileNameFromPath(filename);
		

		logToStream();
		stream_ << func << ' ';

		
	}
	~Logger();

	void formatTime();


	LogStream& stream() { return stream_;  }

	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);
	

	typedef void(*OutputFunc)(const char* msg, int len);
	typedef void(*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);


private:
	void logToStream();

private:
	Timestamp time_;
	LogStream stream_;

	LogLevel level_;
	int line_;
	string basename_;


};


extern Logger::LogLevel g_logLevel; //���� ,�ⲿ����





//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//


 

#define LOG_TRACE  if(Logger::logLevel() <= Logger::TRACE ) \
	Logger(__FILE__, __LINE__, Logger::TRACE, _FUNC_).stream()  //����LogStream ÿ�ζ��½�һ����ʱ��������ʱ����flush

#define LOG_DEBUG if(Logger::logLevel()<=Logger::DEBUG) \
	Logger(__FILE__, __LINE__, Logger::DEBUG, _FUNC_).stream()


#define LOG_INFO if(Logger::logLevel()<=Logger::INFO) \
	 Logger(__FILE__, __LINE__, Logger::INFO, _FUNC_).stream()


#define LOG_OK if(Logger::logLevel()<=Logger::OK) \
	Logger(__FILE__, __LINE__, Logger::OK, _FUNC_).stream()




#define LOG_WARN if(Logger::logLevel()<=Logger::WARN) \
	Logger(__FILE__, __LINE__, Logger::WARN, _FUNC_).stream()

#define LOG_ERROR if(Logger::logLevel()<=Logger::ERROR) \
	Logger(__FILE__, __LINE__, Logger::ERROR, _FUNC_).stream()


#define LOG_FATAL if(Logger::logLevel()<=Logger::FATAL) \
	Logger(__FILE__, __LINE__, Logger::FATAL, _FUNC_).stream()





//logLevel��Ӧ�÷���.cpp�ļ��У���������ʱʹ��lOG_DEBUG������logLevel�޷��������ⲿ���� "  

inline Logger::LogLevel Logger::logLevel()
{
	return g_logLevel;
}


#define CHECK_NOTNULL(val) \
	CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))




// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(string filename, int line, const char *names, T* ptr)
{
	if (ptr == NULL)
	{
		Logger(filename, line, Logger::FATAL,_FUNC_).stream() << names;
	}
	return ptr;
}



NS_END
#endif