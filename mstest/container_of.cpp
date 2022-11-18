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

	struct _test_ tmp;                            //����һ���ṹ�����
	tmp.x = 1;
	tmp.y = 2;
	tmp.z = 3;

	
	struct _test_* p;

	//vc���벻ͨ����gcc����
	p = container_of(&tmp.z, struct _test_, z);
	printf("x:=%d\n", p->x);


}