#include <winsock2.h>
#include <stdio.h>

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

void str_cli(FILE *fp, int sockfd)
{
	char sendline[1024], recvline[1024];

	while (Fgets(sendline, 1024, fp) != NULL)
	{
		Writen(sockfd, sendline, strlen(sendline));

		if (Readline(sockfd, recvline, 1024) == 0)
		{
			exit(0);
		}

		Fputs(recvline, stdout);
	}
}

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	struct hostent *host_entry = gethostbyname("localhost");
	servaddr.sin_addr.s_addr = *(unsigned long*)host_entry->h_addr;

	Connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);

	return 0;
}