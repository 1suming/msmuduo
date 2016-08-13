#include"msmuduo/stdafx.h"
#include"msmuduo/base/echotool.h"

#include<iostream>
int main()
{
	ECHO_OK("ok msg  is ok %d", 5);
	ECHO_ERR("error msg %s", "error");
	ECHO_WARN("warn msg %d", 1);
	ECHO_INFO("info msg %d", 1);
	
	std::cout << "no echo";
}