#include<stdio.h>
#include<iostream>
#include<string>
#include<queue>
#include<thread> //std::thread

#include<windows.h>

#include <mutex>          // std::mutex


std::queue<int> q1;
int counter = 0;
std::mutex mtx;           // locks access to counter


/*
下面的运行有问题
void f1()
{
	while (1)
	{
		counter++;
		mtx.lock();
		q1.push(counter);
		mtx.unlock();
	 

	}
	
}
void f2()
{
	mtx.lock();
	while (!q1.empty())
	{
		 
			int x = q1.front();
			std::cout << x << "," << std::endl;

			q1.pop();
		 
	}
	mtx.unlock();
	

}
*/
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
		mtx.lock(); //如果这里不加锁，可以运行一会儿就退出，具体运行到哪里不确定. 说deque iterator not dereferenable
		if(!q1.empty())
		{

			int x = q1.front();
			std::cout << x << "," << std::endl;

			q1.pop();

		}
		mtx.unlock();
	}
	


}
int main()
{
 
	std::thread t1(f1);
 
	std::thread t2(f2);

	t1.join();
	t2.join();


}