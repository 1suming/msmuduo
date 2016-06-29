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
	//receive ������֤һ���Զ���
	while (true)//����ֻ��Ϊ��WSAEWOULDBLOCKʱѭ��
	{
		//https://msdn.microsoft.com/en-us/library/ms740121(VS.85).aspx
		//recv����������ʱ��otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
		nread= recv(fd, begin() + writerIndex_, writable, 0);

		if (nread < 0 ) //#define SOCKET_ERROR            (-1)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK )//һ���Ƿ����������첽SOCKET�����У�ָ���Ĳ�������������ɣ���˷����������
			{
				continue;
			}
			if (err = WSAEINTR) //ָ���Ĳ���ִ���б�һ���߼������жϣ�����Լ���ִ�У�������һ����LINUX/UNIX�г��֣�WINDOWS��û����
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
	//����û�п���=0�����
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