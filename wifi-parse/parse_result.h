#ifndef __PARSE_RESULT_H__
#define __PARSE_RESULT_H__

#define MAXLINE 1024

typedef struct _wifi_info
{
	char *info_array[5];
	/*char *bssid;
	char *frequency;
	char *signal_level;
	char *flags;
	char *ssid;*/	
	struct _wifi_info *next;
}wifi_info;

typedef enum
{
	SHARE,
	WEP,
	WPA_PSK
}ACCESS_METHOD;

wifi_info *add_info(wifi_info *head, wifi_info *info);

void free_info(wifi_info *head);

wifi_info *parse_result(char *path);

void print_info(wifi_info *head);

char *info_json_encode(wifi_info *head);

#endif