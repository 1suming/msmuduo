#pragma once
/*
noncopyable
*/
namespace ms{


class noncopyable
{
public:
	noncopyable(const noncopyable&) = delete;//c++11 ����Щ�������Ϊ=delete���û�����������
	void operator=(const noncopyable&) = delete;

protected:
	noncopyable() = default;
	~noncopyable() = default;



};
}