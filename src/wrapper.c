#include "wrapper.h"

/*
* DESCRIPTION
* 	connect()包裹函数(下同)
*/
void Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen)
{
	if(connect(sockfd, servaddr, addrlen) < 0){
		perror("connect fail");
		exit(0);
	}
}

/*
*
*/
void Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen)
{
	if(bind(sockfd, myaddr, addrlen) < 0){
		perror("bind error");
		exit(0);
	}
}

/*
* 
*/
void Listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog) < 0){
		perror("listen error");
		exit(0);
	}
}

/*
*
*/
int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
	int connfd;
	if((connfd = accept(sockfd, cliaddr, addrlen)) < 0){
		perror("accept error");
		exit(0);
	}
	return connfd;
}

/*
* DESCRIPTION
* 	
*/
ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;

	while(nleft > 0){
		if((nread = read(fd, ptr, nleft)) < 0){
			if(errno == EINTR)
				nread = 0;
			else
				return (-1);	/*error*/
		} else if(nread == 0) {
			break; 		/*EOF*/
		}
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}

/*
*
*/
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t 	nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;

	while(nleft > 0){
		if((nwritten = write(fd, ptr, nleft)) <= 0){
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;	/*and call write() again*/
			else 
				return(-1); 	/*error*/
		}

		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}

/*
*
*/
ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char 	c, *ptr;

	ptr = vptr;
	for(n = 1; n < maxlen; n++){
	again:
		if((rc = read(fd, &c, 1)) == 1) {
			*ptr++ =c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			*ptr = 0;
			return (n - 1); 	/*EOF, n - 1 bytes were read*/
		} else {
			if(errno == EINTR)
				goto again;
			return(-1);
		}
	}

	*ptr = 0;
	return (n);
}

void *Malloc(size_t size)
{
	void *p;
	p = malloc(size);
	if (p == NULL) {
		perror("malloc error");
		exit(0);
	}
	return p;
}


/*
*
*/
#if 1
Sigfunc *signal(int signo, Sigfunc *func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT; /*SunOS 4.x*/
#endif
	} else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART; /*SVR4, 4.4BSD*/
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
#endif