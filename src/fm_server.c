#include "wrapper.h"
#include "fm_server.h"
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/wait.h> //WNOHANG
#include <unistd.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	int 	listenfd, connfd;
	pid_t 	childpid;
	socklen_t 	clilen;
	
	struct sockaddr_in cliaddr, servaddr;

#if 0
	int 	html_fd;
	if(argc != 2){
		printf("usage: ./serv <html file>");
	}
	html_fd = open(argv[1], O_RDONLY);
	if(html_fd < 0){
		perror("open error.");
		exit(0);
	}
#endif

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	signal(SIGCHLD, sig_chld);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) { /* compare to Accept() in wrapper.c*/
			if(errno == EINTR)
				continue;
			else {
				perror("accept error");
				exit(0);
			}
		}

		if ((childpid = fork()) == 0) {
			close(listenfd);
			printf("in child process %d. connfd: %d\n", childpid, connfd);
			//str_echo(connfd);
			//send_html(connfd);
			//static_file_server(connfd);
			request_handler(connfd);
			exit(0);
		}
		lseek(STDIN_FILENO, 0L, SEEK_SET);
		close(connfd);
	} 
}

/*
*
*/
void str_echo(int fd)
{
	ssize_t n;
	char  	buf[MAXLINE];

again:
	while((n = read(fd, buf, MAXLINE)) > 0)
		writen(fd, buf, n);

	if(n < 0 && errno == EINTR)
		goto again;
	else if (n < 0){
		printf("str_echo: read error\n");
		exit(0);
	}
}

/*
* FUNCTION
* 	request_info *parse_request_info(char *buf, size_t size)
* DESCRIPTION
* 	parse the request info receive from client
*/
request_info *parse_request_info(char *buf, size_t size)
{
	char delimiter[] = "\r\n";
	char *pstr;
	int 	i, offset;
	request_info *info = NULL;

	info = (request_info *)Malloc(sizeof(request_info));
	info->quest = NULL;
	info->param = NULL;
	pstr = strtok(buf, delimiter);
	if (pstr != NULL) {
		offset = 0;
		while(pstr[offset] == ' ') offset++;
		pstr += offset;
		if (memcmp(GET, pstr, strlen(GET)) == 0) {
			info->method = get;
			pstr += strlen(GET);
		}
		else if (memcmp(POST, pstr, strlen(POST)) == 0) {
			info->method = post;
			pstr += strlen(POST);
		}
		else {
			info->method = null;
			goto exit;
		}

		while (*pstr++ != '/') {
			offset++;
			if(offset >= size) goto exit;
		}
		i = 0;
		while (pstr[i] != ' ' && pstr[i] != '?') {				//find the quest string
			i++;
			if (i + offset >= size) goto exit;
		}
		if (i == 0) {
			//info->quest = NULL;
			info->request = NONE; 
			goto exit;
		}
		info->quest = Malloc(i+1);
		strncpy(info->quest, pstr, i);
		info->quest[i] = '\0';

		if (memcmp(info->quest, "file/", 5) == 0) {
			info->request = STATIC_FILE;
		}
		else if (memcmp(info->quest, "login", strlen(info->quest)) == 0) {
			info->request = LOGIN;
		}
		else if (memcmp(info->quest, "setting", strlen(info->quest)) == 0) {
			info->request = SETTING;
		}
		else if (memcmp(info->quest, "wifi_info", strlen(info->quest)) == 0) {
			info->request = WIFI_INFO;
		}
		else if (memcmp(info->quest, "set_wifi", strlen(info->quest)) == 0) {
			info->request = SET_WIFI;
		}
		else if (memcmp(info->quest, "set_age", strlen(info->quest)) == 0) {
			info->request = SET_AGE;
		}
		else {
			info->request = NONE;
		}
		if (pstr[i] == '?') {			//quest has a parameter
			pstr += (i+1);
			offset += i;
			i = 0;
			while (pstr[i] != ' ') {
				i++;
				if (i + offset >= size) goto exit;
			}
			if(i == 0) goto exit;
			info->param = Malloc(i + 1);
			strncpy(info->param, pstr, i);
			info->param[i] = '\0';
			printf("param: %s\n", info->param);
		}
	}

exit:
	return info;
}

/*
*
*/
void request_handler(int connfd)
{
	fd_set rset;
	char buf[BUFSIZ] = {0};
	int n;
	request_info *info;

	FD_ZERO(&rset);
	while (1) {
		FD_SET(connfd, &rset);

		select(connfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(connfd, &rset)) {
			n = read(connfd, buf, BUFSIZ);
			printf("recv: %s\n", buf);
			info = parse_request_info(buf, n);
			break;
		}
	}

	/* 根据请求信息作出回应 */
	switch (info->request) {
		case NONE:
			send_file(connfd, INDEX_HTML);
			break;
		case STATIC_FILE: 
			static_file_server(connfd, info);
			break;
		case LOGIN:
			login_check(connfd, info);
			break;
		case SETTING:

		case WIFI_INFO:
			
		case SET_WIFI:

		case SET_AGE:
		default:
			send_file(connfd, INDEX_HTML);
			break;
	}

}

/*
*
*/
void send_string(int connfd, char *buf)
{
	writen(connfd, buf, strlen(buf));

}

/*
*
*/
void login_check(int connfd, request_info *info)
{
	char *pstr;
	char user[LOGIN_MAXLEN] = {0};
	char pass[LOGIN_MAXLEN] = {0};
	char *token;
	int i;

	if (info->param == NULL) goto wrong_info;

	//pstr = (char *)Malloc(strlen(info->param) + 1);
	if ((pstr = strstr(info->param, "account=")) != NULL) {
		pstr += strlen("account=");
		i = 0;
		while (*pstr != '&' && i < LOGIN_MAXLEN) {
			user[i++] = *pstr++;
		}
		user[i] = '\0';

		if((pstr = strstr(pstr, "pass=")) != NULL) {
			pstr += strlen("pass=");
			i = 0;
			while (*pstr != '\0' && i < LOGIN_MAXLEN) {
				pass[i++] = *pstr++;
			}
			pass[i] = '\0';
		}
		if (compare_user_info(user, pass)) {
			send_file(connfd, SETTING_HTML);
			return;
		}
		else
			goto wrong_info;
	}
	else goto wrong_info;

wrong_info: 
	send_file(connfd, LOGIN_ERR);
}

static int compare_user_info(char *user, char *pass)
{
	char buf[LOGIN_MAXLEN<<2 +1] = {0};
	char cob[LOGIN_MAXLEN<<2 +1] = {0};
	int m, n;
	int fd;

	if (access(USER_INFO, R_OK) >= 0) {
		fd = open(USER_INFO, O_RDONLY);
		n = read(fd, buf, LOGIN_MAXLEN<<2 +1);
		m = sprintf(cob, "%s\n%s", user, pass);
		if (m == n) {
			if(memcmp(buf, cob, n) == 0)
				return 1;
			else 
				return 0;
		}
		else 
			return 0;
	}
	else {
		if(strcmp(DEF_USER, user) == 0 &&
				strcmp(DEF_PASS, pass) == 0) {
			return 1;
		}
		else 
			return 0;
	}
}

/*
*
*/
void send_file(int connfd, const char *path)
{
	char buf[BUFSIZ] = {0};
	int n;
	int fd;

	if (path == NULL) {
deft:				//default
		if (access(DEFAULT_FILE, R_OK) >= 0) {
			fd = open(DEFAULT_FILE, O_RDONLY);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				writen(connfd, buf, n);
			}
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
		else {
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
	}
	else {
		if (access(path, R_OK) < 0) {			//not readable
			printf("cant access file\n");
			goto deft;
		}
		else {
			fd = open(path, O_RDONLY);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				writen(connfd, buf, n);
			}
				
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
	}

exit:

	if ((n = read(connfd, buf, BUFSIZ)) != 0) {
		printf("%s", buf);
		goto exit;
	}
	//free(file_name);
	//free(path);
	close(fd);
}

#if 0
void static_file_server(int connfd, request_info *info)
{
	fd_set rset;
	char buf[BUFSIZ] = {0};
	char *pst = NULL;
	int i;
	char *file_name;
	char *path = NULL;
	char *pstr;
	int n, length;

	int fd;
#if 0
	request_info *info;

	FD_ZERO(&rset);
	while (1) {
		FD_SET(connfd, &rset);

		select(connfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(connfd, &rset)) {
			n = read(connfd, buf, BUFSIZ);
			printf("recv: %s\n", buf);
			info = parse_request_info(buf, n);
			break;
#if 0
			pstr = strstr(buf, "GET");
			if (pstr != NULL) {
				pstr = strstr(pstr, "/");
				if(pstr != NULL)
					pstr++;
					i = 0;
					while (pstr[i] != ' ') {
						i++;
						if (i > n)
							continue;
					}
					break;
			}
#endif
		}
	}
#endif
	if (info->quest == NULL) {
deft:				//default
		if (access(DEFAULT_FILE, R_OK) >= 0) {
			fd = open(DEFAULT_FILE, O_RDONLY);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				writen(connfd, buf, n);
			}
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
		else {
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
	}
	else {
 		path = (char *)Malloc(strlen(SOURCE_DIR)+strlen(info->quest)+1);
		strcpy(path, SOURCE_DIR);
		strcat(path, info->quest);
		printf("path: %s\n", path);
		if (access(path, R_OK) < 0) {			//not readable
			printf("cant access file\n");
			goto deft;
		}
		else {
			fd = open(path, O_RDONLY);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				writen(connfd, buf, n);
			}
				
			shutdown(connfd, SHUT_WR);
			goto exit;
		}
	}

exit:

	if ((n = read(connfd, buf, BUFSIZ)) != 0) {
		printf("%s", buf);
		goto exit;
	}
	//free(file_name);
	free(path);
	close(fd);
}
#else
void static_file_server(int connfd, request_info *info)
{
	fd_set rset;
	char buf[BUFSIZ] = {0};
	char *path = NULL;
	int n;

	int fd;

	if (info->quest == NULL) {
			goto exit;
	}
	else {
 		path = (char *)Malloc(strlen(SOURCE_DIR)+strlen(info->quest)+1);
		strcpy(path, SOURCE_DIR);
		strcat(path, info->quest);
		printf("path: %s\n", path);
		if (access(path, R_OK) < 0) {			//not readable
			printf("cant access file\n");
			goto exit;
		}
		else {
			fd = open(path, O_RDONLY);
			while ((n = read(fd, buf, BUFSIZ)) > 0) {
				writen(connfd, buf, n);
			}
			goto exit;
		}
	}

exit:

	shutdown(connfd, SHUT_WR);
	if ((n = read(connfd, buf, BUFSIZ)) != 0) {
		printf("%s", buf);
		goto exit;
	}
	//free(file_name);
	free(path);
	close(fd);
}
#endif

/*
*
*/
void send_html(int connfd)
{
	ssize_t n;
	fd_set 	rset;
	char 	buf[MAXLINE] = {0};
	char ready = 0;
	int maxfd;

	FD_ZERO(&rset);
	while(1) {
		FD_SET(STDIN_FILENO, &rset);
		FD_SET(connfd, &rset);
		maxfd = max(STDIN_FILENO, connfd);

		select(maxfd + 1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &rset)){
			if((n = read(STDIN_FILENO, buf, MAXLINE)) > 0)
				writen(connfd, buf, n);
			else{
				shutdown(connfd, SHUT_WR);	/* send FIN*/
				//break;
			}
			//printf("readable. n= %d", n);
		}

		if(FD_ISSET(connfd, &rset)){
			if((n = read(connfd, buf, MAXLINE)) == 0){
				break;
			}
			else
				printf("%s", buf);
		}
	}
}

void sig_chld(int signo)
{
	pid_t 	pid;
	int 	stat;

	//pid = wait(&stat);
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminted\n", pid);
	return;
}
