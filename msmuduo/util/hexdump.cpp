#include"msmuduo/util/hexdump.h"

namespace hexdump
{
	void hexdump(const char* buf, int len)
	{
		/*�� register �ؼ��ʻ���ʾ������������������ᾭ����ʹ�á����������C++��׼��ע��˵�������ʾ���Ա����ԡ����仰˵���ڱ�׼C++�У�register �ؼ��ʲ�Ӱ������ʵ�ʹ��ܡ�C++11�У� register �ؼ��ʵ��÷���������deprecated
		*/
		register int m;
		register int n;
		register const char* data;
		data = buf;

		for (m = 0; m<len; m++)
		{
			if ((!(m % 2)) && (m != 0)) fprintf(stderr, " ");//ÿ2��char�м���һ���ո�
			if ((!(m % 20)) && (m != 0))
			{
				for (n = 20; n>0; n--)
				{
					if ((*(data + m - n) > 31) && (*(data + m - n) < 127))
						fprintf(stderr, "%c", *(data + m - n));
					else
						fprintf(stderr, ".");
				}
				//ÿ20��char��Ҫ����
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