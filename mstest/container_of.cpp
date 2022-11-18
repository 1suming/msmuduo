#include<stdio.h>


struct _test_
{
	char  x;
	int  y;
	float z;
};
#define offsetof(TYPE,MEMBER) ( (size_t)&( ((TYPE*)0)->MEMBER) )



#define container_of(ptr, type, member) ({   \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})



int main()
{
	int temp = -1;
	temp = offsetof(struct _test_, z);
	printf("temp=%d\n", temp);

	struct _test_ tmp;                            //定义一个结构体变量
	tmp.x = 1;
	tmp.y = 2;
	tmp.z = 3;

	
	struct _test_* p;

	//vc编译不通过，gcc可以
	p = container_of(&tmp.z, struct _test_, z);
	printf("x:=%d\n", p->x);


}