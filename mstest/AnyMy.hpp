#ifndef _ANY_hpp
#define _ANY_hpp
/*
http://www.cnblogs.com/findumars/p/5006183.html
http://www.cnblogs.com/sld666666/p/3965147.html
*/


//#include<boost/any.hpp>

#include<iostream>

#define CURR_NS_BEGIN namespace ANYMY {
#define CURR_NS_END }
#define CURR_NS_USING using namespace ANYMY

CURR_NS_BEGIN



//  //泛型数据容器holder的非泛型基类     

struct IHolder{
	virtual ~IHolder()
	{}

	virtual const std::type_info& type() const = 0;
	virtual IHolder* clone() const = 0;

};

template<typename ValueType>
class Holder :public IHolder
{
public:
	Holder(const ValueType& val) :value_(val)
	{

	}
	virtual const std::type_info& type() const
	{
		return typeid(ValueType);
	}

	virtual IHolder* clone() const
	{
		return new Holder(value_);
	}

public:
	ValueType value_;
};

//--------------------------------------
 
class Any
{
public:
	Any() :holder_(NULL)
	{

	}

	template<typename ValueType>
	Any(const ValueType& val) :
		holder_(new Holder<ValueType>(val))
	{

	}
	Any(const Any& other) :
		holder_(other.holder_ ? other.holder_->clone() : NULL)
	{

	}

	~Any()
	{
		if (holder_)
		{
			delete holder_;
			holder_ = NULL;
		}
	}

	bool empty()
	{
		return !holder_;
	}
	const std::type_info& type() const {
		return holder_ ? holder_->type() : typeid(void);
	}

	Any& operator=(Any rhs){
		return swap(rhs);
	}

	template<typename ValueType>
	Any& operator=(const ValueType& val){
		return Any(val).swap(*this);
	}

	IHolder* getHolder(){
		return holder_;
	}
private:
	Any& swap(Any& rhs){
		std::swap(holder_, rhs.holder_);
		return *this;
	}



private:
	IHolder* holder_;

};


template<typename ValueType>
ValueType* any_cast(Any* val)
{
	return (val && val->type() == typeid(ValueType) )
		? &static_cast<Holder<ValueType>*>(val->getHolder())->value_ : 0;

}

class BadAnyCast :public std::bad_cast
{
public:
	virtual const char * what() const throw()
	{
		return "mb::badAnyCast: "
			"failed conversion using mb::badAnyCast";
	}
};

template<typename ValueType>
ValueType any_cast(Any& val){
	ValueType* rtn = any_cast<ValueType>(&val);
	if (!rtn)boost::throw_exception(BadAnyCast());
	return *rtn;
}

CURR_NS_END
#endif