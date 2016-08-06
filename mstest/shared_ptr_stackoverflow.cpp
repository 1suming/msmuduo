#include<boost/shared_ptr.hpp>
#include<iostream>
using namespace std;

/*
http://blog.csdn.net/zcczcw/article/details/6449545
按F5调试会出现:
0x00333977 (msvcr120d.dll) (mstest.exe 中)处有未经处理的异常:  0xC00000FD: Stack overflow (参数:  0x00000000, 0x001D2000)。

c++析构函数

1.析构函数会在对象生命期结束时自动被调用
2。含有成员变量的类在自身析构函数调用结束后，将按照成员变量声明顺序的逆序依次调用其析构函数。
3.具有继承关系的类在自身析构函数调用结束后，将按照基类声明顺序的逆序依次调用其析构函数。\
\



下面的也会出现stackv overflow
void func()
{

char stack_overflow[1024 * 1024 * 5];
stack_overflow[0] = 1;
}


*/
struct Message
{
	Message(int index = 0)
	{
		m_index = index;
	}
	~Message() // 析构函数
	{
		cout << "~Message: " << m_index << endl;
	}

	shared_ptr<Message> m_pNext; // 指向消息队列中的下一个消息 
	int m_index;
};


int main(int argc, char *argv[])
{
	shared_ptr<Message> pHead = shared_ptr<Message>(new Message(0));
	shared_ptr<Message> pCur = pHead;

	for (int i = 1; i < 2000; i++)// 这里构建一个长度为2000的消息队 列
	{
		shared_ptr<Message> pNext = shared_ptr<Message>( new Message(i));
		pCur->m_pNext = pNext;
		pCur = pNext;
	}
	/*
	 断开消息队列
	 这是我最后采用的方案，析构时的迭代之所以会发生，是源于boost::shared_ptr的引用计数原理，只要将消息链断开就不会有这样的问题 。针对前面的测试例子，只要在return 0前面加上如下代码即可避免“Stack Overflow”：
	 pCur = pHead;
	 for(i=1; i<2000; ++i) // 依次断 开消息链
	 {
	 pHead = pCur->m_pNext;
	 pCur->m_pNext.reset();
	 pCur = pHead;
	 }
	 
	 */
	/*修改方法
	pCur = pHead;
	for (int i = 1; i < 2000; i++)
	{
		pHead = pCur->m_pNext;
		pCur->m_pNext.reset();
		pCur = pHead;
	}
	*/
	return 0;
}