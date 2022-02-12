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
    ͨ��release��������ȡԪ�ص�����Ȩ
�����´��룬����p��ֱ�ӻ�ȡ��Tests�е�һ��Ԫ�ص�����Ȩ����ôTests�����о�ֱ��ɾ���˵�һ��Ԫ�ء�
*/

    boost::ptr_vector<A>::auto_type p = ptrVec.release(ptrVec.begin());

    cout << p->i << endl;


    return 0;
}