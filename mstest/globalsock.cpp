#include"globalsock.h"

int make_socket_nonblocking(int fd)
{
#ifdef WIN32

	unsigned long nonblocking = 1;
	if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR)
	{
		cout << "ioctlsocket error,fd:" << fd;
		return -1;
	}

#else 
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0)
	{
		return -1;
	}
	if (!(flags & O_NONBLOCK))
	{
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			return -1;
		}
	}

#endif

	return 0;
}
int  createServerSocket(int port)
{
	struct sockaddr_in LocalAddr;
	//Create Socket
	int ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET)
	{
		cout << "Create Socket Failed::" << GetLastError() << endl;
		return -1;
	}
	memset(&LocalAddr, 0, sizeof LocalAddr);
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = INADDR_ANY;
	LocalAddr.sin_port = htons(port);


	//Bind Socket
	int Ret = bind(ServerSocket, (struct sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if (Ret != 0)
	{
		cout << "Bind Socket Failed::" << GetLastError() << endl;
		return -1;
	}
	//listen
	Ret = listen(ServerSocket, 10);
	if (Ret != 0)
	{
		cout << "listen Socket Failed::" << GetLastError() << endl;
		return -1;
	}
	cout << "serverSocket" << ServerSocket << endl;

	return ServerSocket;
}