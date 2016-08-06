#include<boost/shared_ptr.hpp>
#include<iostream>
using namespace std;

/*
http://blog.csdn.net/zcczcw/article/details/6449545
��F5���Ի����:
0x00333977 (msvcr120d.dll) (mstest.exe ��)����δ��������쳣:  0xC00000FD: Stack overflow (����:  0x00000000, 0x001D2000)��

c++��������

1.�����������ڶ��������ڽ���ʱ�Զ�������
2�����г�Ա�������������������������ý����󣬽����ճ�Ա��������˳����������ε���������������
3.���м̳й�ϵ���������������������ý����󣬽����ջ�������˳����������ε���������������\
\



�����Ҳ�����stackv overflow
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
	~Message() // ��������
	{
		cout << "~Message: " << m_index << endl;
	}

	shared_ptr<Message> m_pNext; // ָ����Ϣ�����е���һ����Ϣ 
	int m_index;
};


int main(int argc, char *argv[])
{
	shared_ptr<Message> pHead = shared_ptr<Message>(new Message(0));
	shared_ptr<Message> pCur = pHead;

	for (int i = 1; i < 2000; i++)// ���ﹹ��һ������Ϊ2000����Ϣ�� ��
	{
		shared_ptr<Message> pNext = shared_ptr<Message>( new Message(i));
		pCur->m_pNext = pNext;
		pCur = pNext;
	}
	/*
	 �Ͽ���Ϣ����
	 �����������õķ���������ʱ�ĵ���֮���Իᷢ������Դ��boost::shared_ptr�����ü���ԭ��ֻҪ����Ϣ���Ͽ��Ͳ��������������� �����ǰ��Ĳ������ӣ�ֻҪ��return 0ǰ��������´��뼴�ɱ��⡰Stack Overflow����
	 pCur = pHead;
	 for(i=1; i<2000; ++i) // ���ζ� ����Ϣ��
	 {
	 pHead = pCur->m_pNext;
	 pCur->m_pNext.reset();
	 pCur = pHead;
	 }
	 
	 */
	/*�޸ķ���
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