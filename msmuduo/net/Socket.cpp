#include"msmuduo/stdafx.h"
#include"Socket.h"

#include"base/sockettool.h"
#include"base/Logging.h"
#include"net/InetAddress.h"

NS_USING;


Socket::~Socket()
{
	sockettool::close(sockfd_); //RAII
}
/*
bool Socket::getTcpInfo(struct tcp_info* tcpi) const
{
	socklen_t len = sizeof(*tcpi);
	bzero(tcpi, len);
	return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}


bool Socket::getTcpInfoString(char* buf, int len) const
{
	struct tcp_info tcpi;
	bool ok = getTcpInfo(&tcpi);
	if (ok)
	{
		snprintf(buf, len, "unrecovered=%u "
			"rto=%u ato=%u snd_mss=%u rcv_mss=%u "
			"lost=%u retrans=%u rtt=%u rttvar=%u "
			"sshthresh=%u cwnd=%u total_retrans=%u",
			tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
			tcpi.tcpi_rto,          // Retransmit timeout in usec
			tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
			tcpi.tcpi_snd_mss,
			tcpi.tcpi_rcv_mss,
			tcpi.tcpi_lost,         // Lost packets
			tcpi.tcpi_retrans,      // Retransmitted packets out
			tcpi.tcpi_rtt,          // Smoothed round trip time in usec
			tcpi.tcpi_rttvar,       // Medium deviation
			tcpi.tcpi_snd_ssthresh,
			tcpi.tcpi_snd_cwnd,
			tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
	}
	return ok;
}
*/

void Socket::bindAddress(const InetAddress& addr)
{
	sockettool::bindOrDie(sockfd_, addr.getSockAddrInet());

}
void Socket::listen()
{
	sockettool::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	int connfd = sockettool::accept(sockfd_, &addr);
	if (connfd > 0)
	{
		peeraddr->setSockAddrInet(addr);
	}

	return connfd;
}

void Socket::shutdownWrite()
{
	LOG_INFO << "----------shutdownWrite:";
	sockettool::shutdownWrite(sockfd_);
}
void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
		(char*)&optval, static_cast<socklen_t>(sizeof optval));
	//FIXME check

}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
		(char*)&optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0)
	{
		LOG_SYSERR << "SO_REUSEPORT failed.";
	}
#else
	if (on)
	{
		LOG_ERROR << "SO_REUSEPORT is not supported.";
	}
#endif
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&optval, static_cast<socklen_t>(sizeof optval));
}