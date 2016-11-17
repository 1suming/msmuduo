#ifndef _Session_H
#define _Session_H

#include"Item.h"

#include<msmuduo/stdafx.h>
#include<msmuduo/base/Logging.h>
#include<msmuduo/net/TcpConnection.h>


#include<msmuduo/base/StringPiece.h>

#include<boost/bind.hpp>
#include<boost/enable_shared_from_this.hpp>
#include<boost/noncopyable.hpp>
#include<boost/tokenizer.hpp>


using ms::StringPiece;
using namespace ms;

class MemcacheServer;

class Session : boost::noncopyable,
	public boost::enable_shared_from_this < Session >
{
public:
	Session(MemcacheServer* owner, const ms::TcpConnectionPtr& conn)
		: owner_(owner),
		conn_(conn),
		state_(kNewCommand),
		protocol_(kAscii), // FIXME
		noreply_(false),
		policy_(Item::kInvalid),
		bytesToDiscard_(0),
		needle_(Item::makeItem(kLongestKey, 0, 0, 2, 0)),
		bytesRead_(0),
		requestsProcessed_(0)
	{
		conn_->setMessageCallback(
			boost::bind(&Session::onMessage, this, _1, _2, _3));
	}
	~Session()
	{
		LOG_INFO << "requests processed: " << requestsProcessed_
			<< " input buffer size: " << conn_->inputBuffer()->internalCapacity()
			<< " output buffer size: " << conn_->outputBuffer()->internalCapacity();
	}




private:
	enum State {
		kNewCommand,
		kReceiveValue,
		kDiscardValue,
	};
	enum Protocol
	{
		kAscii,
		kBinary,
		kAuto,
	};

	void onMessage(const ms::TcpConnectionPtr& conn,
		ms::Buffer* buf,
		ms::Timestamp);
	void onWriteComplete(const ms::TcpConnectionPtr& conn);
	void receiveValue(ms::Buffer* buf);
	void discardValue(ms::Buffer* buf);
	// TODO: highWaterMark
	// TODO: onWriteComplete

	// returns true if finished a request
	bool processRequest(StringPiece request);
	void resetRequest();
	void reply(StringPiece msg);



	struct SpaceSeparator
	{
		void reset() {}
		template <typename InputIterator, typename Token>
		bool operator()(InputIterator& next, InputIterator end, Token& tok);
	};

	typedef boost::tokenizer<SpaceSeparator,
		const char*,
		StringPiece> Tokenizer;
	struct Reader;
	bool doUpdate(Tokenizer::iterator& beg, Tokenizer::iterator end);
	void doDelete(Tokenizer::iterator& beg, Tokenizer::iterator end);



	MemcacheServer* owner_;
	ms::TcpConnectionPtr conn_;
	State state_;
	Protocol protocol_;


	//current request
	string command_;
	bool noreply_;
	Item::UpdatePolicy policy_;
	ItemPtr currItem_;
	size_t bytesToDiscard_;


	//cached
	ItemPtr needle_;
	ms::Buffer outputBuf_;


	//per session stats
	size_t bytesRead_;
	size_t requestsProcessed_;

	static string kLongestKey;



};

typedef boost::shared_ptr<Session> SessionPtr;


#endif