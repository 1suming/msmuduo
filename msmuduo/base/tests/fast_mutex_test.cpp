 
#include"msmuduo/stdafx.h"
#include<queue>

#include<iostream>

#include"base/fast_mutex.h"

#include<thread>//std::thread

NS_USING;


queue<int> q1;
int counter = 0;

typedef fast_mutex MY_MUTEX;

MY_MUTEX mtx;           // locks access to counter

 
void f1()
{
	while (1)
	{
		mtx.lock();
		counter++;

		q1.push(counter);

		mtx.unlock();


	}

}
void f2()
{
	while (1)
	{
		//mtx.lock(); //如果这里不加锁，可以运行一会儿就退出，具体运行到哪里不确定. 说deque iterator not dereferenable
		if (!q1.empty())
		{

			int x = q1.front();
			 cout << x << "," << endl;

			q1.pop();

		}
		//mtx.unlock();
	}



}
int main()
{

	std::thread t1(f1);

	std::thread t2(f2);

	t1.join();
	t2.join();


}