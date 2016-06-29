#ifndef _fileutil_h
#define _fileutil_h


#include "StringPiece.h"
#include <boost/noncopyable.hpp>

NS_BEGIN

namespace FileUtil
{


	//not thread safe
	class AppendFile : boost::noncopyable //�����ֻ�ǵ����Ĵ��ļ���д������
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
	��ȡ�ļ�����Ŀ¼����, ����׺��.exe
	C:/a/1.txt ->1.txt
	C:/a/  ->a
	*/
	string getFileNameFromPath(const char* filePath);
	string getFileNameFromPath(const string& filePath);

	/*
	��ȡ�ļ�����Ŀ¼���ƣ�������׺
	*/
	string getFileNameNotExtFromPath(const char* filePath);
	string getFileNameNotExtFromPath(const string& filePath);



}




NS_END

#endif