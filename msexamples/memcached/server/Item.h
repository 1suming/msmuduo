#ifndef _Item_h
#define _Item_h

#include<msmuduo/stdafx.h>
#include<msmuduo/base/Atomic.h>
#include<msmuduo/base/StringPiece.h>



#include<boost/make_shared.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

using std::string;
using ms::StringPiece;
 
namespace ms {
	class Buffer;
}
class Item;

typedef boost::shared_ptr<Item> ItemPtr;//TODO: use unique_ptr
typedef boost::shared_ptr<const Item> ConstItemPtr;

//Item is immutable once added into hash table 
class Item : boost::noncopyable
{
public:
	enum UpdatePolicy
	{
		kInvalid,
		kSet,
		kAdd,
		kReplace,
		kAppend,
		kPrepend,
		kCas,
	};
	//set 
	/*<command name> <key> <flag> <expire> <bytes>
<data block>  
*/
	static ItemPtr makeItem(StringPiece keyArg,
		uint32_t flagsArg,
		int exptimeArg,
		int valuelen,
		uint64_t casArg)
	{
		return boost::make_shared<Item>(keyArg, flagsArg, exptimeArg, valuelen, casArg);
		//return ItemPtr(new Item(keyArg, flagsArg, exptimeArg, valuelen, casArg));
	}

	Item(StringPiece keyArg,
		uint32_t flagsArg,
		int exptimeArg,
		int valuelen,
		uint64_t casArg);

	~Item()
	{
		::free(data_);
	}
	StringPiece key() const
	{
		return StringPiece(data_, keylen_);
	}

	uint32_t flags() const
	{
		return flags_;
	}

	int rel_exptime() const
	{
		return rel_exptime_;
	}

	const char* value() const
	{
		return data_ + keylen_;
	}

	size_t valueLength() const
	{
		return valuelen_;
	}

	uint64_t cas() const
	{
		return cas_;
	}

	size_t hash() const
	{
		return hash_;
	}

	void setCas(uint64_t casArg)
	{
		cas_ = casArg;
	}

	size_t neededBytes() const
	{
		return totalLen() - receivedBytes_;
	}

	void append(const char* data, size_t len);

	bool endsWithCRLF() const
	{
		return receivedBytes_ == totalLen()
			&& data_[totalLen() - 2] == '\r'
			&& data_[totalLen() - 1] == '\n';
	}

	void output(ms::Buffer* out, bool needCas = false) const;

	void resetKey(StringPiece k);








private:
	//Key和value的总长度
	int totalLen() const { return keylen_ + valuelen_; }

	int keylen_;
	const uint32_t flags_;
	const int rel_exptime_;
	const int valuelen_;

	int receivedBytes_;//FIXME: remove this member
	uint64_t     cas_;
	size_t       hash_;
	char*        data_;
	/*
	CASValue casValue = client.gets(key);
	//*****
	//本地的各种处理
	//*****
	CASResponse response = client.cas(key, newValue, casValue);
	//在我取数据时item的版本号是casValue.getCas()，所以提交时我期望item的版本号是没有改变过的。如果被修改过，不是我取数据时的版本号，
	那么Memcached server对这次提交什么也不做，返回true或false由用户自己来提出解决方案(什么也不做或是重新获取版本号，再次重试提交等)  
	
	*/

};



#endif