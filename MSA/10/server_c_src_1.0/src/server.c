#include	"unp_my.h"

#define	MAXREQLINE 16384			/* max req line header length */
#define	MAXANS 700000				/* max #bytes client can request */
#define MAXMETHODNAME 25
#define MAXPATH 512


int getSubStr(char *,char *,int);
void formatPath(char *);

void
err_sys(const char *fmt, ...)
{
	/*
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	*/
	puts(fmt);
	exit(1);
}


void
err_quit(const char *fmt, ...)
{
	//va_list		ap;

	//va_start(ap, fmt);
	//err_doit(0, LOG_ERR, fmt, ap);
	puts(fmt);
	//va_end(ap);
	exit(1);
} 

/* include Listen */
void
Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_sys("listen error");
}
/* end Listen */

int
tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one */

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		Close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	/* errno from final socket() or bind() */
		err_sys("tcp_listen error for %s, %s", host, serv);

	Listen(listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(listenfd);
}
/* end tcp_listen */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */

int
Tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(tcp_listen(host, serv, addrlenp));
}


void
Close(int fd)
{
	if (close(fd) == -1)
		err_sys("close error");
}

pid_t
Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1)
		err_sys("fork error");
	return(pid);
} 

void *
Malloc(size_t size)
{
	void	*ptr;

	if ( (ptr = malloc(size)) == NULL)
		err_sys("malloc error");
	return(ptr);
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
} 


void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(fd, level, optname, optval, optlen) < 0)
		err_sys("setsockopt error");
}


void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		/* printf("child %d terminated\n", pid); */
	}
	return;
}


int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	void				sig_chld(int), sig_int(int), web_child(int,long);
	socklen_t			clilen, addrlen;
	struct sockaddr		*cliaddr;
	long nReqCounter;

	if (argc == 2)
		listenfd = Tcp_listen(NULL, argv[1], &addrlen);
	else if (argc == 3)
		listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	else
		err_quit("usage: serv01 [ <host> ] <port#>");
	cliaddr = Malloc(addrlen);

	Signal(SIGCHLD, sig_chld);
	Signal(SIGINT, sig_int);

	nReqCounter=0;
	for ( ; ; ) {
		clilen = addrlen;
		if ( (connfd = accept(listenfd, cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}

		nReqCounter++;
		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			web_child(connfd,nReqCounter);	/* process the request */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
/* end serv01 */

/* include sigint */
void
sig_int(int signo)
{
	//void	pr_cpu_time(void);

	//pr_cpu_time();
	//printf("\nuser time = <HERE NO DATA sig_int %d>\n",signo); 
	printf("\nEXIT\n");
	exit(0);
}
/* end sigint */


static ssize_t
my_read(int fd, char *ptr)
{
	static int	read_cnt = 0;
	static char	*read_ptr;
	static char	read_buf[MAXLINE];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	int		n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
} 


void
web_child(int sockfd,long nReqCnt)
{
	int bFirst=1;
	long header_size;
	int len;
	int offset;
	int			ntowrite;
	ssize_t		nread;
	char		line[MAXREQLINE], sMsgRespBody[MAXANS];
	char sMsgRespHeader[512];
	char buf[256];
	char sMethod[MAXMETHODNAME+1];
	char sPath[MAXPATH+1];
	char sMsgTempl[]="HTTP/1.1 200 OK\r\n"
				"Server: My simple server\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: %ld\r\n"
				"\r\n";
	char sMsgTemplJSON[]="HTTP/1.1 200 OK\r\n"
				"Server: My simple server\r\n"
				"Content-Type: application/json\r\n"
				"Content-Length: %ld\r\n"
				"\r\n";
	
	strcpy(sMsgRespBody,"REQUEST:\r\n///\r\n");
	header_size=0;

	for ( ; ; ) {
		if ( (nread = Readline(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

		if (bFirst)
		{
			printf("%ld REQ: ",nReqCnt);
			bFirst=0;
			
			offset=getSubStr(line,sMethod,MAXMETHODNAME);
			if (offset<0) 
			{
				puts("WRONG METHOD!");
				Close(sockfd);
				return;
			}
			if (getSubStr(line+offset+1,sPath,MAXPATH) <0)
			{
				puts("WRONG PATH!");
				Close(sockfd);
				return;				
			}
			printf("%s %s\n",sMethod,sPath);
		}
		// printf("%s",line); // debug
		len=strlen(line);
		if ((len==2)&&(line[len-1]=='\n'))
		{
			formatPath(sPath); 
			
			sprintf(buf,"/// HEADER=%ld bytes\r\n",header_size);
			//printf("%s",buf); // debug
			strcat(sMsgRespBody,buf);
			
			if (strcmp(sPath,"/")==0)
			{
				sprintf(sMsgRespHeader,sMsgTempl,strlen(sMsgRespBody));
				Writen(sockfd, sMsgRespHeader, strlen(sMsgRespHeader));
				Writen(sockfd, sMsgRespBody, strlen(sMsgRespBody));
			}
			else if(strcmp(sPath,"/health")==0)
			{
				strcpy(sMsgRespBody,"{\"status\" : \"OK\"}");
				sprintf(sMsgRespHeader,sMsgTemplJSON,strlen(sMsgRespBody));
				Writen(sockfd, sMsgRespHeader, strlen(sMsgRespHeader));
				Writen(sockfd, sMsgRespBody, strlen(sMsgRespBody));
			}
			else if(strcmp(sPath,"/metrics")==0)
			{
				strcpy(sMsgRespBody,"# TYPE connections_total counter\r\n");
				sprintf(sMsgRespBody,"%sconnections_total %ld\r\n",sMsgRespBody,nReqCnt);
				sprintf(sMsgRespHeader,sMsgTempl,strlen(sMsgRespBody));
				Writen(sockfd, sMsgRespHeader, strlen(sMsgRespHeader));
				Writen(sockfd, sMsgRespBody, strlen(sMsgRespBody));
			}
			
			Close(sockfd);
			return;
		}
		header_size+=len;
		strcat(sMsgRespBody,line);
		//
	}
}


int getSubStr(char *line,char *sRes,int nLimit)
{
	int i;
	
	i=0;
    while(line[i] != '\0')
	{
		if( (line[i] == ' ') || (line[i] == '\r') || (line[i] == '\n') ) break;
        if((i+1)>nLimit) return -1;
		sRes[i]=line[i];
		i++;
    }
	sRes[i]='\0';
	
	return i;
}


void formatPath(char sPath[])
{
	int len;
	
	len=strlen(sPath);
	if( (len>1) && (sPath[len-1]=='/')) sPath[len-1]='\0';
	
	return;
}
