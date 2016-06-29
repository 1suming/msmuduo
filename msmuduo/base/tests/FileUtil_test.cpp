#include"../../stdafx.h"
#include"../FileUtil.h"
#include<iostream>

NS_USING;


int main()
{
	string result;
	int64_t size = 0;

	string filename;
#ifdef WIN
	filename = "C:/fileutil.log";
#else
	filename = "/fileutil.log";
#endif
	FileUtil::AppendFile  file(filename);

	string str;
	str = "helloworld";
	file.append(str.c_str(), str.length());
	str = "this is ok";
	file.append(str.c_str(), str.length());

	file.flush();
	 
}