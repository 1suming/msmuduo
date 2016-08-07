#include"msmuduo/stdafx.h"
#include"Buffer.h"
#include"base/sockettool.h"
#include"base/Logging.h"
#include<iostream>

NS_USING;

static const size_t kCheapPrepend = 8;
static const size_t kInitialSize = 1024;


const char Buffer::kCRLF[] = "\r\n";


ssize_t Buffer::readFd(int fd, int* savedErrno)
{

#ifdef WIN

	/* 下面的是有recv，效率不过WSARecv，类似linux下面的readv
	const size_t writable=writableBytes();

	int retReadCnt=0;
	int nread = 0;
	//receive ，不保证一次性读完
	while (true)//这里只是为了WSAEWOULDBLOCK时循环
	{
		//https://msdn.microsoft.com/en-us/library/ms740121(VS.85).aspx
		//recv当发生错误时，otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
		nread= recv(fd, begin() + writerIndex_, writable, 0);

		if (nread < 0 ) //#define SOCKET_ERROR            (-1)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK )//一般是非阻塞或者异步SOCKET操作中，指定的操作不能立即完成，因此返回这个代码
			{
				continue;
			}
			if (err = WSAEINTR) //指定的操作执行中被一个高级调用中断，你可以继续执行，但是这一般在LINUX/UNIX中出现，WINDOWS上没见过
			{
				continue;
			}
			else
			{
				*savedErrno = WSAGetLastError();
				break;
			}
		}
		else
		{
			retReadCnt += nread;
			writerIndex_ += nread;
			break;
		}
	}
	 

	return retReadCnt;
	*/
	//WSARecv https://msdn.microsoft.com/zh-cn/library/ms741688(en-us,VS.85).aspx
	/*
	int WSARecv(
	__in          SOCKET s,
	__in_out      LPWSABUF lpBuffers,
	__in          DWORD dwBufferCount,
	__out         LPDWORD lpNumberOfBytesRecvd,
	__in_out      LPDWORD lpFlags,
	__in          LPWSAOVERLAPPED lpOverlapped,
	__in          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine

	返回值
	 If no error occurs and the receive operation has completed immediately, WSARecv returns zero.
	In this case, the completion routine will have already been scheduled to be called once the calling thread is in the alertable state. 
	Otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
	The error code WSA_IO_PENDING indicates that the overlapped operation has been successfully initiated and 
	that completion will be indicated at a later time. Any other error code indicates that the overlapped operation was not successfully initiated and no completion indication will occur.
	*/

	char extrabuf[65536];
	const size_t writable = writableBytes();

 	WSABUF wsabuf[2];
	DWORD recvBytes, flags;
	flags = 0;

	wsabuf[0].buf=begin()+writerIndex_ ;
	wsabuf[0].len=writable;
	wsabuf[1].buf=extrabuf;
	wsabuf[1].len=sizeof extrabuf;

	int retReadCnt = 0;
	int nread = 0;
	int rc;
	//while (1)
	{
		/*
		之前存在的问题，这个readFd要返回读取的字节数，如果返回是0代表连接关闭，会处理handleClose事件。在windows上面我之前是直接返回retReadCnt
		有个例子，服务器一次性发送大量数据到客户端，客户端的readFd返回0导致在函数void TcpConnection::handleRead(Timestamp receiveTime)
		handleClose，导致客户端没有接收到数据进行处理。
		一次性接受到大量数量，我打印WSALAstError结果如下：
		WSAGetLastError 0,The operation completed successfully.

		最后找到原因是漏掉了retReadCnt += recvBytes; 导致return retReadCnt是0，连接就被关闭了。


		为什么linux readFd返回0代表连接关闭 没有问题？ 因为我们能够的readFd是被handleRead调用的，能走到handleRead就代表有数据读或者错误发生。
		*/
		//cout << endl << "_____" << endl;
		rc = WSARecv(fd, wsabuf, 2, &recvBytes, &flags, NULL, NULL);
		 		 
		LOG_DEBUG << "rc:" << rc << ",recbbytes:" <<(uint32_t) recvBytes;

		int err;
		if (rc <0  )//#define SOCKET_ERROR            (-1) //==SOCKET_ERROR && (WSA_IO_PENDING!=(err=WSAGetLastError()) )  
		{
			err = WSAGetLastError();
			*savedErrno = err;
			LOG_DEBUG << "err" << err << "," << getErrorMsg(err);
			if (err == WSAEWOULDBLOCK)//一般是非阻塞或者异步SOCKET操作中，指定的操作不能立即完成，因此返回这个代码，经过试验，确实有errno:10035
			{
				//continue; 不用循环，会一直循环
			}
			if (err = WSAEINTR) //指定的操作执行中被一个高级调用中断，你可以继续执行，但是这一般在LINUX/UNIX中出现，WINDOWS上没见过
			{
				//continue;
			}
			else
			{
 				//break;
			}
		}
		else
		{
 			if (recvBytes <= writable)  //这里一定要判断
			{
				retReadCnt += recvBytes;
				writerIndex_ += recvBytes;

			}
			else
			{
				retReadCnt += recvBytes; //这个一定不能漏
				writerIndex_ = buffer_.size();
				append(extrabuf, recvBytes - writable);
			}
			
			//break;
		}


	}
	return retReadCnt;
	



#else

	//saved an ioctl()/FIONREAD call to tell how much to read
	char extrabuf[65536];
	struct iovec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;

	//when there is enough space in this buffer, don't read into extrabuf
	// when extrabuf is used, we read 128k-1 bytes at most.

	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
	const ssize_t n = ::readv(fd, vec, iovcnt);
	/*
	为什么没有反复调用read直到返回EAGAIN，因为采用的是level trigger
	*/
	//下面没有考虑=0的情况
	if (n < 0)
	{
		*savedErrno=errno; 
	}
	else if(implicit_cast<size_t>(n)<=writable)
	{
		writerIndex_ += n;
	}
	else 
	{
		writerIndex_ =buffer_.size();
		append(extrabuf,n-writable);
		
	}

	return n;


#endif
}