#ifndef _CString_h
#define _CString_h

#include<iostream>
using namespace std;

/*
https://msdn.microsoft.com/zh-cn/library/dd293665.aspx
https://msdn.microsoft.com/zh-cn/library/dd293668.aspx
*/
class CString
{
public:
	explicit CString(size_t len):
		len_(len),
		data_(new char[len])
	{
		cout<< "construct CString(size_t len)" << endl;
	}
	explicit CString(const char* str) 
		 
	{
 		int len = strlen(str);
		data_ = new char[len];
		std::copy(str, str+len, data_);
		len_ = len;

		cout << "construct 	explicit CString(const char* str) " << endl;
	}

	~CString()
	{
		cout << "destructor" << endl;
		if (data_ != nullptr)
		{
			cout << "delete resouce" << endl;
			delete[] data_;//Ϊʲô��delete[],��Ϊnew����һ������


		}
	}
	 
	CString(const CString& rhs):
		len_(rhs.len_),
		data_(new char[rhs.len_])
	{
		cout << "���ƹ��캯��" << endl;
		std::copy(rhs.data_, rhs.data_ + rhs.len_, data_);
	}
	 

	/*
	�ƶ����캯��
	*/
	CString(CString&& rhs) :
		data_(nullptr),
		len_(0)
	{
		//move����ı���֮������������Ҳ��һ���֣���͵������һ����ֵ�����еĶ��ڴ�ͨ��ָ��ķ�ʽת�Ƶ���һ����������͵�ĵ���Ӧ�ó�����
		cout << "move contruct" << endl;
		data_ = rhs.data_;
		len_ = rhs.len_;

		//��Դ��������ݳ�Ա�����Ĭ��ֵ���������Է�ֹ�������
		rhs.data_ = nullptr;
		rhs.len_ = 0;
	}

private:
	size_t len_;
	char* data_;
};
#endif