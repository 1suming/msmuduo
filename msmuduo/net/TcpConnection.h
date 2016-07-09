#ifndef _TcpConnection_h
#define _TcpConnection_h

#include"base/StringPiece.h"


#include"net/Callbacks.h"
#include"net/Buffer.h"
#include"net/InetAddress.h"


#include<boost/scoped_ptr.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/enable_shared_from_this.hpp>
#include<boost/noncopyable.hpp>
#include<boost/any.hpp>


NS_BEGIN


class Channel;
class EventLoop;
class Socket;

/*
Tcp Connection,for both client and server usage;�ͻ��˺�server�˹���
TcpConnecion��ΨһĬ��ʹ��shared_ptr�������class���̳�enable_shared_from_this��

TcpConnection��ʾ�ġ�һ��TCP���ӡ������ǲ��������ģ�һ�����ӶϿ������TcpConnection�����ûɶ���ˡ�

(����ͻ��˶Ͽ���ĳ��Tcp socket������Ӧ�ķ���˽����е�TcpConnection�������Ȼ�Ǹ�heap����
��������stack���󣩵�����Ҳ�����ߵ���ͷ��������ʱ���ǲ�������delete���������Ϊ�����ط����ܻ�����
�������ã�óȻdelete����ɿ���ָ�롣ֻ��ȷ�������ط�û�г��иö��������ʱ�����ܰ�ȫ�����ٶ���



//this is an interface class, so don't expose too much details.
*/

class TcpConnection :boost::noncopyable,
	public boost::enable_shared_from_this<TcpConnection>
{
public:

	//construct a TcpConnection with a connected sockfd
	///Use should not create this object
	TcpConnection(EventLoop* loop,
		const string& nameArg,
		int sockfd,
		const InetAddress& localAddr,
		const InetAddress& peerAddr);

	~TcpConnection();

	EventLoop* getLoop() const { return loop_; }
	const string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }

	bool connected() const { return state_ == kConnected; }

	// return true if success.
	//bool getTcpInfo(struct tcp_info*) const;
	//string getTcpInfoString() const;

	//void send(string&& message); // C++11
	void send(const void*message, int len);

	//�����loop thread,��ֱ�ӷ��ͣ�������runInLoop
	void send(const StringPiece& message);


	// void send(Buffer&& message); // C++11
	void send(Buffer* message);//this one will swap data
	

	const char* stateToString() const;

	//Not Thread safe,no simultaneous calling
	void shutdown();
	// void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
	void forceClose();
	void forceCloseWithDelay(double seconds);


	void setTcpNoDelay(bool on);


	void setContext(const boost::any& context)
	{context_ = context;}

	const boost::any& getContext() const
	{return context_;}

	boost::any* getMutableContext()
	{return &context_;}

	/*--------------------Callback register ----------------------*/
	void setConnectionCallback(const ConnectionCallback& cb)
	{
		connectionCallback_ = cb;
	}

	void setMessageCallback(const MessageCallback& cb)
	{
		messageCallback_ = cb;
	}

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{
		writeCompleteCallback_ = cb;
	}

	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
	{
		highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark;
	}


	/*-------------------------------------------------------------------*/

	//Advanced interface
	Buffer* inputBuffer()
	{
		return &inputBuffer_;
	}
	Buffer* outputBuffer()
	{
		return &outputBuffer_;
	}

	///Internal use only
	void setCloseCallback(const CloseCallback& cb)
	{closeCallback_ = cb;}

	//called when TcpServer accepts a new connection
	void connectEstablished(); //should be called only once 
	//called when TcpServer has removed me from its map 
	void connectDestroyed();//should be called only once







private:
	enum StateE //����״̬
	{
		kDisconnected,
		kConnecting,
		kConnected,
		kDisconnecting
	};

	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();

	// void sendInLoop(string&& message);
	void sendInLoop(const StringPiece& message);
	void sendInLoop(const void* message, size_t len);
	void shutdownInLoop();

	// void shutdownAndForceCloseInLoop(double seconds);
	void forceCloseInLoop();
	void setState(StateE s) { state_ = s; }


	EventLoop* loop_;
	const string name_;
	StateE state_; //��������ǳ���Ҫ��FIXME:use atomic variable 
	//we don't expose those classes to client 
	boost::scoped_ptr<Socket> socket_; //ӵ��Socket,������ʱ������close socket
	boost::scoped_ptr<Channel> channel_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;

	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;


	size_t highWaterMark_; 



	Buffer inputBuffer_;
	Buffer outputBuffer_; //use list<Buffer> as output buffer.

	boost::any context_;

	//FIXME creatTime,lastReceiveTime_,byteReceived_,byteSend_

 


};


//!Important: shared_ptr
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;


NS_END
#endif