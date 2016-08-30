#include"msmuduo/util/hexdump.h"

namespace hexdump
{
	void hexdump(const char* buf, int len)
	{
		/*外 register 关键词还提示编译器，这个变量将会经常被使用。但紧随其后，C++标准就注释说，这个提示可以被忽略。换句话说，在标准C++中，register 关键词不影响程序的实际功能。C++11中， register 关键词的用法被废弃（deprecated
		*/
		register int m;
		register int n;
		register const char* data;
		data = buf;

		for (m = 0; m<len; m++)
		{
			if ((!(m % 2)) && (m != 0)) fprintf(stderr, " ");//每2个char中间有一个空格
			if ((!(m % 20)) && (m != 0))
			{
				for (n = 20; n>0; n--)
				{
					if ((*(data + m - n) > 31) && (*(data + m - n) < 127))
						fprintf(stderr, "%c", *(data + m - n));
					else
						fprintf(stderr, ".");
				}
				//每20个char就要换行
				fprintf(stderr, "\n");
			}
			fprintf(stderr, "%02x", *(data + m));
		}
		for (m = 0; m < (20 - ((len % 20) ? (len % 20) : 20)) * 2 + (10 - ((len % 20) ? (len % 20 - 1) : 19) / 2); m++)
			fprintf(stderr, " ");

		for (m = len - ((len % 20) ? (len % 20) : 20); m<len; m++)
		{
			if ((*(data + m)>31) && (*(data + m) < 127))
				fprintf(stderr, "%c", *(data + m));
			else
				fprintf(stderr, ".");
		}


		fprintf(stderr, "\n\n");
	}
}