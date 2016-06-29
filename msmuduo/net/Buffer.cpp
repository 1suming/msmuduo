#include"../stdafx.h"
#include"Buffer.h"

NS_USING;

static const size_t kCheapPrepend = 8;
static const size_t kInitialSize = 1024;


const char Buffer::kCRLF[] = "\r\n";


ssize_t Buffer::readFd(int fd, int* savedErrno)
{

#ifdef WIN

	
	const size_t writable=writableBytes();

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
			writerIndex_ += nread;
			break;
		}
	}
	 

	return nread;
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