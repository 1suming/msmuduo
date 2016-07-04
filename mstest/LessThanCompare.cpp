#include<string>
#include<iostream>
#include<memory> //unique_ptr
#include<assert.h>
#include<boost/operators.hpp>

#include"LessThanCompare.h"

using namespace std;


class Point : boost::less_than_comparable<Point> //С�ڹ�ϵ��˽�м̳�
{
public:
	explicit Point(int a = 0, int b = 0, int c = 0) :x(a), y(b), z(c)
	{

	}
	void print() const
	{
		cout << x << "," << y  <<","<<z<<endl;
	}

	friend bool operator<(const Point& l, const Point&  r) //��Ԫ��������������޹� //:point�ඨ����һ����Ԫoperator<��������Ȼ�������>��<=��>=����less_than_comparable�Զ�����
	{
		return (l.x*l.x + l.y*l.y + l.z*l.z <
			r.x*r.x + r.y*r.y + r.z*r.z);

	}
private:
	int x, y, z;
};
 



void test()
{
	//Point p0, p1(1, 2, 3), p2(3, 0, 5), p3(3, 2, 1);

	Point2 p0, p1(1, 2, 3), p2(3, 0, 5), p3(3, 2, 1);


	assert(p0 < p2 && p1 < p2);
	assert(p2 > p0);
	assert(p1 <= p3);
	assert(!(p1<p3) && !(p1>p3));

}

int main()
{

}