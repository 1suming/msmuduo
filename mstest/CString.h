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
			delete[] data_;//为什么用delete[],因为new的是一个数组


		}
	}
	 
	CString(const CString& rhs):
		len_(rhs.len_),
		data_(new char[rhs.len_])
	{
		cout << "复制构造函数" << endl;
		std::copy(rhs.data_, rhs.data_ + rhs.len_, data_);
	}
	 

	/*
	移动构造函数
	*/
	CString(CString&& rhs) :
		data_(nullptr),
		len_(0)
	{
		//move语义的本质之处，归纳起来也就一个字：“偷”。将一个右值对象中的堆内存通过指针的方式转移到另一个对象，则是偷的典型应用场景。
		cout << "move contruct" << endl;
		data_ = rhs.data_;
		len_ = rhs.len_;

		//将源对象的数据成员分配给默认值，这样可以防止析构多次
		rhs.data_ = nullptr;
		rhs.len_ = 0;
	}

private:
	size_t len_;
	char* data_;
};
#endif