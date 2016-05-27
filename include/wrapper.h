#ifndef __WRAPPER_H__
#define __WRAPPER_H__
#include <fcntl.h>
#include <sys/types.h> 	/*size_t ssize_t*/
#include <errno.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <signal.h>

#define MAXLINE 1024
#define SERV_PORT 7415 
#define LISTENQ	20

typedef void Sigfunc(int);

void Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
void Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
void Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);

void *Malloc(size_t size);

Sigfunc *signal(int signo, Sigfunc *func);

#endif
