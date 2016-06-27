#include <winsock2.h>
#include <stdio.h>

#ifndef WIN32 
#pragma  error "Ö»Ö§³Öwindows"
#endif

#ifdef win32 
#pragma comment(lib,"ws2_32.lib")

#endif
int Socket(int family, int type, int protocol)
{
	int n;

	if ((n = socket(family, type, protocol)) < 0)
	{
		fputs("socket error", stderr);
	}

	return(n);
}

void Connect(int fd, const struct sockaddr *sa, int salen)
{
	if (connect(fd, sa, salen) < 0)
	{
		fputs("connect error", stderr);
	}
}

void Send(int fd, const void *ptr, size_t nbytes, int flags)
{
	if (send(fd, (const char*)ptr, nbytes, flags) != nbytes)
	{
		fputs("send error", stderr);
	}
}

void Bind(int fd, const struct sockaddr *sa, int salen)
{
	if (bind(fd, sa, salen) < 0)
	{
		fputs("bind error", stderr);
	}
}

void Listen(int fd, int backlog)
{
	if (listen(fd, 5) < 0)
	{
		fputs("listen error", stderr);
	}
}

int Accept(int fd, struct sockaddr *sa, int *salenptr)
{
	int        n;
again:

	if ((n = accept(fd, sa, salenptr)) < 0)
	{
		if (GetLastError() == WSAECONNABORTED)
		{
			goto again;
		}
		else
		{
			fputs("accept error", stderr);
		}
	}

	return(n);
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int n;

	if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
	{
		int err = WSAGetLastError();
		fputs("select error", stderr);
	}

	return(n);                /* can return 0 on timeout */
}

char* Fgets(char *ptr, int n, FILE *stream)
{
	char* rptr;

	if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
	{
		fputs("fgets error", stderr);
	}

	return (rptr);
}

void Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
	{
		fputs("fputs error", stderr);
	}
}

int writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	int        nwritten;
	const char* ptr;

	ptr = (const char*)vptr;
	nleft = n;

	while (nleft > 0)
	{
		if ((nwritten = send(fd, ptr, nleft, 0)) <= 0)
		{
			if (nwritten < 0 && WSAGetLastError() == WSAEINTR)
			{
				nwritten = 0; /* and call write() again */
			}
			else
			{
				return(-1);        /* error */
			}
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return(n);
}

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
	{
		fputs("writen error", stderr);
	}
}

static int        read_cnt;
static char        *read_ptr;
static char        read_buf[1024];

static int my_read(int fd, char *ptr)
{
	if (read_cnt <= 0)
	{
	again:
		if ((read_cnt = recv(fd, read_buf, sizeof(read_buf), 0)) < 0)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				goto again;
			}

			return(-1);
		}
		else if (read_cnt == 0)
		{
			return(0);
		}

		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

int readline(int fd, void *vptr, size_t maxlen)
{
	int        n, rc;
	char c, *ptr;

	ptr = (char*)vptr;

	for (n = 1; n < maxlen; n++)
	{
		if ((rc = my_read(fd, &c)) == 1)
		{
			*ptr++ = c;

			if (c == '\n')
			{
				break; /* newline is stored, like fgets() */
			}
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return(n - 1); /* EOF, n - 1 bytes were read */
		}
		else
		{
			return(-1);        /* error, errno set by read() */
		}
	}

	*ptr = 0; /* null terminate like fgets() */
	return(n);
}

int readlinebuf(void **vptrptr)
{
	if (read_cnt)
	{
		*vptrptr = read_ptr;
	}

	return(read_cnt);
} /* end readline */

int Readline(int fd, void *ptr, size_t maxlen)
{
	int        n;

	if ((n = readline(fd, ptr, maxlen)) < 0)
	{
		fputs("readline error", stderr);
	}

	return(n);
}

void Close(int fd)
{
	if (closesocket(fd) == -1)
	{
		fputs("close error", stderr);
	}
}

void str_echo(int sockfd)
{
	int        n;
	char buf[1024];

again:
	while ((n = recv(sockfd, buf, 1024, 0)) > 0)
	{
		Writen(sockfd, buf, n);
	}

	if (n < 0 && WSAGetLastError() == WSAEINTR)
	{
		goto again;
	}
	else if (n < 0)
	{
		fputs("str_echo: read error", stderr);
	}
}

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	int        i, maxi, maxfd, listenfd, connfd, sockfd;
	int        nready, client[FD_SETSIZE];
	int n;
	fd_set rset, allset;
	char buf[1024];
	int        clilen;
	struct sockaddr_in        cliaddr, servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	Bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));
	Listen(listenfd, 5);

	maxfd = listenfd; /* initialize */
	maxi = -1; /* index into client[] array */

	for (i = 0; i < FD_SETSIZE; i++)
	{
		client[i] = -1;        /* -1 indicates available entry */
	}

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	fputs("server start:", stdout);

	for (;;)
	{
		rset = allset; /* structure assignment */
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset))
		{        /* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr*)&cliaddr, &clilen);

			for (i = 0; i < FD_SETSIZE; i++)
			{
				if (client[i] < 0)
				{
					client[i] = connfd;        /* save descriptor */
					break;
				}
			}

			if (i == FD_SETSIZE)
			{
				fputs("too many clients", stderr);
			}

			FD_SET(connfd, &allset); /* add new descriptor to set */

			if (connfd > maxfd)
			{
				maxfd = connfd; /* for select */
			}

			if (i > maxi)
			{
				maxi = i; /* max index in client[] array */
			}

			if (--nready <= 0)
			{
				continue; /* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++)
		{        /* check all clients for data */
			if ((sockfd = client[i]) < 0)
			{
				continue;
			}

			if (FD_ISSET(sockfd, &rset))
			{
				if ((n = recv(sockfd, buf, 1024, 0)) == 0)
				{
					/*4connection closed by client */
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
				else
				{
					Writen(sockfd, buf, n);
				}

				if (--nready <= 0)
				{
					break; /* no more readable descriptors */
				}
			}
		}
	}
}