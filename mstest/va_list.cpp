#include<stdio.h>
#include<stdarg.h>

void func(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	while (*fmt)
	{
		switch (*fmt)
		{
		case 'd':
			fprintf(stdout, "%d\n", (int)va_arg(ap, int));
			break;
		case 'c':
			fprintf(stdout, "%c\n", (char)va_arg(ap, char));
			break;

		case 's':
			fprintf(stdout, "%s\n", (char*)va_arg(ap, char*));
			break;

		default:
			fprintf(stderr, "error fmt\n");

		}
		fmt++;
	}
	va_end(ap);

}
/*my implement */
typedef char* va_list_my;//不要定义成void*
#define va_start_my(ap,lastarg) (ap = (va_list)&lastarg + sizeof(lastarg))
 
#define va_arg_my(ap, t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) ) //由于内存对齐，不能直接用sizeof  (*(t *)((ap += sizeof(t)) - sizeof(t))) // // 
 



#define va_end_my(ap) (ap=(va_list_my)0)

void funcMy(char *fmt, ...)
{
	va_list_my ap;
	va_start_my(ap, fmt);

	while (*fmt)
	{
		switch (*fmt)
		{
		case 'd':
			fprintf(stdout, "%d\n", (int)va_arg(ap, int));
			break;
		case 'c':
			fprintf(stdout, "%c\n", (char)va_arg_my(ap, char));
			break;

		case 's':
		 
			fprintf(stdout, "%s\n",  (char*)va_arg_my(ap, char*));
			break;

		default:
			fprintf(stderr, "error fmt\n");

		}
		fmt++;
	}
	va_end_my(ap);

}

int main()
{
	funcMy("dcs", 10, 's', "hello world");

}