#ifndef _Logging_h
#define _Logging_h

#include"Timestamp.h"
#include"LogStream.h"
#include"Thread.h"
#include"FileUtil.h"

#include"echotool.h"

NS_BEGIN

#undef ERROR //ewindows下ERROR在某个文件中有宏定义

class T;
class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,

		INFO,
		OK,//显示为绿色

		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

	Logger(string filename, int line, LogLevel level):
		//basename_(filename),
		line_(line),
		level_(level),

		time_(Timestamp::now()) //构造时赋值time_
	{
		basename_ = FileUtil::getFileNameFromPath(filename);
		logToStream();

	}
	Logger(string filename, int line, LogLevel level,const char* func): //传入的filename是__LINE__ 
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


extern Logger::LogLevel g_logLevel; //声明 ,外部变量





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
	Logger(__FILE__, __LINE__, Logger::TRACE, _FUNC_).stream()  //返回LogStream 每次都新建一个临时对象，析构时调用flush

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





//logLevel不应该放在.cpp文件中，否则链接时使用lOG_DEBUG出：现logLevel无法解析的外部符号 "  

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