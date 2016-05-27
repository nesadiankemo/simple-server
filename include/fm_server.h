#ifndef __FM_SERVER_H__
#define __FM_SERVER_H__
#define SOURCE_DIR 	"./src/"
#define DEFAULT_FILE "./src/login.html"
#define LOGIN_ERR 	"./src/login_err.html"
#define USER_INFO 	"./user_info"
#define SETTING_HTML "./src/setting.html"
#define INDEX_HTML 	"./src/login.html"
#define DEF_USER 	"admin"
#define DEF_PASS 	"admin"
#define GET 	"GET"
#define POST 	"POST"

#define LOGIN_MAXLEN 30

#define max(a, b) \
	({ __typeof__(a) _a = (a); \
	 __typeof__(b) _b = (b); \
	 _a > _b ? _a : _b; })

typedef enum 
{
	get,
	post,
	null
}METHOD;

typedef enum
{
	NONE,
	STATIC_FILE,
	LOGIN,
	SETTING,
	WIFI_INFO,
	SET_WIFI, 
	SET_AGE
}REQUEST;

typedef struct _request_info
{
	METHOD method;
	REQUEST request;
	char *quest;
	char *param;
}request_info;

void request_handler(int connfd);
void str_echo(int fd);
void sig_chld(int sigo);
void send_html(int connfd);
void static_file_server(int connfd, request_info *info);
void send_file(int connfd, const char *path);
void login_check(int connfd, request_info *info);
static int compare_user_info(char *user, char *pass);



#endif