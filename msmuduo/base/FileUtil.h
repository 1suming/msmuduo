#ifndef _fileutil_h
#define _fileutil_h


#include "StringPiece.h"
#include <boost/noncopyable.hpp>

NS_BEGIN

namespace FileUtil
{


	//not thread safe
	class AppendFile : boost::noncopyable //这个类只是单纯的打开文件，写入内容
	{
	public:
		explicit AppendFile(StringArg filename);

		~AppendFile();


		void append(const char* logline, const size_t len);

		void flush();

		size_t writtenBytes() const { return writtenBytes_; }


	private:

		size_t write(const char* logline, size_t len);

		FILE* fp_;
		char buffer_[64 * 1024];
		size_t writtenBytes_;

	};

	/*
	获取文件名或目录名称, 待后缀如.exe
	C:/a/1.txt ->1.txt
	C:/a/  ->a
	*/
	string getFileNameFromPath(const char* filePath);
	string getFileNameFromPath(const string& filePath);

	/*
	获取文件名或目录名称，不带后缀
	*/
	string getFileNameNotExtFromPath(const char* filePath);
	string getFileNameNotExtFromPath(const string& filePath);



}




NS_END

#endif