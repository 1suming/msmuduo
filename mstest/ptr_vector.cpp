#include<iostream>
#include<boost/ptr_container/ptr_vector.hpp>

using namespace std;

class A
{
public:
    A(int i) :i(i)
    {
        std::cout << "construct" << i <<  std::endl;
    }
    ~A()
    {
        std::cout << "destruct " << i <<  std::endl;
    }

    int i;
};

int main()
{
    boost::ptr_vector<A> ptrVec;
    ptrVec.push_back(new A(1));
    ptrVec.push_back(new A(2));
    /*
    通过release方法来获取元素的所有权
见如下代码，变量p就直接获取了Tests中第一个元素的所有权，那么Tests容器中就直接删除了第一个元素。
*/

    boost::ptr_vector<A>::auto_type p = ptrVec.release(ptrVec.begin());

    cout << p->i << endl;


    return 0;
}