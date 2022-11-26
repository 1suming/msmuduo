#pragma once
/*
noncopyable
*/
namespace ms{


class noncopyable
{
public:
	noncopyable(const noncopyable&) = delete;//c++11 见这些函数标记为=delete调用会产生编译错误
	void operator=(const noncopyable&) = delete;

protected:
	noncopyable() = default;
	~noncopyable() = default;



};
}