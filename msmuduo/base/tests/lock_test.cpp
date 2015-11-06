#include"../../stdafx.h"
#include"../lock.h"
#include"../Thread.h"

#include <boost/bind.hpp>
#include<boost/ptr_container/ptr_vector.hpp>
#include<queue>
using namespace ms;

int g_count = 0;//全局资源

mutex_t mutex;

void add1(int threadNo)
{
	lock_guard_t lockguard(mutex);
	sleep(10);//some work should to do
	g_count++;
	sleep(0);
	printf("Thread nO:%d,count:%d\n", threadNo, g_count);
}
void locktest()
{
	boost::ptr_vector<Thread> vec;
	for (int i = 1; i <= 5; i++)
	{
		Thread *t = new Thread(boost::bind(add1, i));
		t->start();
		vec.push_back(t);

		//Thread t(boost::bind(add1, i));
		//t.start();
		//t.join();
	}
	for (int i = 0; i < vec.size(); i++)
	{
		vec[i].join();
	}
	printf("last count:%d\n", g_count);
	sleep(1000);//milliseconds

}
std::queue<int> g_queue;
condition_var_t cond(mutex);
void condFunc1()
{
	lock_guard_t lock(mutex);
	while(g_queue.empty())
	{
		cond.wait();

	}
	int x = g_queue.front();
	g_queue.pop();
	printf("condFunc1 execute!,it's %d\n",x);


}
void conditionTest()
{
	Thread t(condFunc1);
	t.start();
	printf("has called condFunc1\n");

	printf("g_queue add \n");
	g_queue.push(5);
	cond.notify();
	t.join();


}
int main()
{
	conditionTest();

}