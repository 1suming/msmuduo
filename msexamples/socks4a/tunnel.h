#ifndef _tunnel_h
#define _tunnel_h


#include"msmuduo/base/Logging.h"
#include"msmuduo/net/EventLoop.h"
#include"msmuduo/net/InetAddress.h"
#include"msmuduo/net/TcpClient.h"
#include"msmuduo/net/TcpServer.h"


using namespace ms;

class Tunnel : public boost::enable_shared_from_this<Tunnel>,
	boost::noncopyable
{
public:
	Tunnel(EventLoop* loop,
		const InetAddress& serverAddr,
		const TcpConnectionPtr& serverConn):

		client_(loop, serverAddr, serverConn->name()),
		serverConn_(serverConn)
	{
		LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort()
			<< " <-> " << serverAddr.toIpPort();

	}
	~Tunnel()
	{
		LOG_INFO << "~Tunnel";
	}

	void setup()
	{
		client_.setConnectionCallback(
			boost::bind(&Tunnel::onClientConnection, shared_from_this(), _1));

		client_.setMessageCallback(
			boost::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
	 
		//Ϊʲô��weak_ptr

		serverConn_->setHighWaterMarkCallback(
			boost::bind(&Tunnel::onHighWaterMarkWeak, boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
			10 * 1024 * 1024);


	}
	void teardown()
	{
		client_.setConnectionCallback(defaultConnectionCallback);
		client_.setMessageCallback(defaultMessageCallback);

		if (serverConn_)
		{
			serverConn_->setContext(boost::any());
			serverConn_->shutdown();
		}
	}


	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
		// serverConn_.reset();
	}



	void onClientConnection(const TcpConnectionPtr& conn)
	{
		LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			conn->setTcpNoDelay(true);
			
			//connҲ������onHighWaterMarkWeak
			conn->setHighWaterMarkCallback(
				boost::bind(&Tunnel::onHighWaterMarkWeak, boost::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
				10 * 1024 * 1024);

			//serverConn_�ǿͻ��˵��м��˵�conn
			serverConn_->setContext(conn);//�����conn���м��˵���������conn

			if (serverConn_->inputBuffer()->readableBytes() >= 0) //�����ӵ���ʱ�򣬿�����֮ǰ�ͻ��˷�������û�з��ͣ��������﷢��
			{
				conn->send(serverConn_->inputBuffer()); //������send ��Buffer*)��buffer�� buf->retrieveAll();
			}


	
		}
		else
		{
			teardown(); //
		}

	}
	//server->�м��� 
	void onClientMessage(const TcpConnectionPtr& conn,
		Buffer* buf, Timestamp time)
	{
		LOG_DEBUG << conn->name() << " " << buf->readableBytes();

		if (serverConn_)
		{
			//�м���--���ͻ���
			serverConn_->send(buf);

		}
		else
		{
			buf->retrieveAll();
			abort();
		}

	}
	void onHighWaterMark(const TcpConnectionPtr& conn, size_t bytesToSent)
	{
		LOG_INFO << "onHighWaterMark " << conn->name()
			<< " bytes " << bytesToSent;

		disconnect();

	}
	static void onHighWaterMarkWeak(const boost::weak_ptr<Tunnel>& wkTunnel,
		const TcpConnectionPtr& conn,
		size_t bytesToSent)
	{
		boost::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
		if (tunnel)
		{
			tunnel->onHighWaterMark(conn, bytesToSent);
		}
	}

private:
	TcpClient client_;
	TcpConnectionPtr serverConn_;//�ͻ��˵��м��˵�����
};

//��������ָ��TunnelPtr

typedef boost::shared_ptr<Tunnel> TunnelPtr;

#endif