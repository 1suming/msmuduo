#include<string>
#include<iostream>

#include<boost/ptr_container/ptr_vector.hpp>

using namespace std;

class A{
public:
	A(string name="hello")
	{
		name_ = name;
	}
	~A()
	{
		cout << "A desctor" << endl;
	}
	
	string name_;
};
void test()
{

	boost::ptr_vector<A> vec;

	vec.push_back(new A());
	vec.push_back(new A("hello_2"));

	typedef boost::ptr_vector<A>::auto_type APtr; //把指针从容器中删除,并返回这个指针. auto_type是数据的智能指针,类似于std::auto_ptr.
      

	APtr  aptr = vec.pop_back();
	cout << aptr->name_ << endl;

	/*
	Finally, you can now also "copy" and "assign" an auto_type ptr by calling move():

	boost::ptr_vector<T>::auto_type move_ptr = ...;
	return boost::ptr_container::move( move_ptr );
	*/
	
	APtr aptr2= boost::ptr_container::move(aptr);
	/*
	APtr aptr2 = aptr;

	 error LNK2019: 无法解析的外部符号 "public: __thiscall boost::ptr_container_detail::static_move_ptr<class A,struct boost::ptr_container_detail::clone_deleter<struct


	 */

	cout << aptr2->name_ << endl;





}
int main()
{
	test();

}