#include"msmuduo/util/hexdump.h"

int main()
{
	char buf[] = "hello134 This is a world";
	hexdump::hexdump(buf,strlen(buf));

}