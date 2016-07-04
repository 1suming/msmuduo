#ifndef _LESSTHANCOMPARE_H
#define _LESSTHANCOMPARE_H

#include<string>
#include<iostream>
#include<memory> //unique_ptr
#include<assert.h>
#include<boost/operators.hpp>

using namespace std;



 
template<class T>
struct MYLessThanComparable 
{
	friend bool operator>=(const T& l, const T& r)
	{
		return !(l < r);
	}
	friend bool operator>(const T& l, const T& r)
	{
		return r < l; //����zע��:r<l,��Ȼl>r
	}
	friend bool operator<= (const T& l, const T& r)
	{
		return !(l>r);
	}


};

class Point2 :MYLessThanComparable<Point2>
{
public:
	explicit Point2(int a = 0, int b = 0, int c = 0) :x(a), y(b), z(c)
	{

	}
	void print() const
	{
		cout << x << "," << y << "," << z << endl;
	}

	friend bool operator<(const Point2& l, const Point2&  r) //��Ԫ��������������޹� //:point�ඨ����һ����Ԫoperator<��������Ȼ�������>��<=��>=����less_than_comparable�Զ�����
	{
		return (l.x*l.x + l.y*l.y + l.z*l.z <
			r.x*r.x + r.y*r.y + r.z*r.z);

	}
private:
	int x, y, z;
};




#endif