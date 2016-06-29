#include"../stdafx.h"
#include"FileUtil.h"
 
#include<boost/static_assert.hpp>

#include<errno.h>
#include<assert.h>
#include<stdio.h>//setbuffer
#ifdef WIN
#include<tchar.h> //_T
#endif
 
#ifdef LINUX 
	#include<sys/stat.h>

#endif
NS_USING;


FileUtil::AppendFile::AppendFile(StringArg filename)
: fp_(::fopen(filename.c_str(), "a")),  // 'e' for O_CLOEXEC ,windows下不可用
writtenBytes_(0)
{
	assert(fp_);
	//::setbuffer(fp_, buffer_, sizeof buffer_); //!windows下setbuffer没有
	 
	// posix_fadvise POSIX_FADV_DONTNEED ?
}

FileUtil::AppendFile::~AppendFile()
{
	::fclose(fp_);
}

void FileUtil::AppendFile::append(const char* logline, const size_t len)
{
	size_t n = write(logline, len);
	size_t remain = len - n;
	while (remain > 0)
	{
		size_t x = write(logline + n, remain);
		if (x == 0)
		{
			int err = ferror(fp_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
			}
			break;
		}
		n += x;
		remain = len - n; // remain -= x
	}

	writtenBytes_ += len;
}

void FileUtil::AppendFile::flush()
{
	::fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char* logline, size_t len)
{
	// #undef fwrite_unlocked
	//muduo用的是fwrite的线程不安全版本::fwrite_unlocked(logline, 1, len, fp_);
	return fwrite(logline, 1, len, fp_);

}

string FileUtil::getFileNameFromPath(const char* filePath)
{
	const char* nameStart;
	const char* nameEnd = filePath + strlen(filePath) - 1;

	//跳过目录名称后连续的/或'\'
	while (nameEnd >= filePath && (*nameEnd == '/' || *nameEnd == '\\'))
	{
		nameEnd--;
	}
	nameStart = nameEnd;
	nameEnd++;

	//定位目录起始名称的位置
	while (nameStart >= filePath)
	{
		if (*nameStart == '/' || *nameStart == '\\')
			break;

		nameStart--;
	}
	nameStart++;

	string filename("");
	if (nameStart < nameEnd)
	{
		filename.assign(nameStart, nameEnd); //[first,last)

	}
	return filename;
}
string FileUtil::getFileNameFromPath(const string& filePath)
{
	return getFileNameFromPath(filePath.c_str());
}
string FileUtil::getFileNameNotExtFromPath(const char* filePath)
{
	const char* nameStart;
	const char* nameEnd = filePath + strlen(filePath) - 1;

	//如果文件是目录
	if (*nameEnd == '/' || *nameEnd == '\\')
	{
		//跳过目录名称后连续的/或'\'
		while (nameEnd >= filePath && (*nameEnd == '/' || *nameEnd == '\\'))
		{
			nameEnd--;
		}
		nameEnd++;

	}
	else
	{
		const char* ptr = nameEnd;
		//找到文件后缀起始位置
		while (ptr >= filePath)
		{
			if (*ptr == '.')
			{
				nameEnd = ptr;
				break;
			}

			if (*ptr == '/' || *ptr == '\\')
				break;

			ptr--;
		}
	}

	nameStart = nameEnd - 1;

	 
	//定位目录起始名称的位置
	while (nameStart >= filePath)
	{
		if (*nameStart == '/' || *nameStart == '\\')
			break;

		nameStart--;
	}
	nameStart++;

	string filename("");
	if (nameStart < nameEnd)
	{
		filename.assign(nameStart, nameEnd); //[first,last)

	}
	return filename;
}
string FileUtil::getFileNameNotExtFromPath(const string& filePath)
{
	return getFileNameNotExtFromPath(filePath.c_str());
}
bool FileUtil::fileExist(const char* filePath)
{
#ifdef WIN 
 
	DWORD attr = GetFileAttributes(filePath); 

	if (attr == INVALID_FILE_ATTRIBUTES)
		return false;
	return true;

#else
	if (!access(filePath),F_OK)
	{
		return true;
	}
	return false;
#endif
}