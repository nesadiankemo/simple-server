#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "parse_result.h"
#include "wrapper.h"

#define TEMP_PATH "./scan_results"
#define BSSID 	"\"bssid\": "
#define FREQ 		"\"frenquency\": "
#define SIG_LEVEL "\"signal_level\": "
#define FLAGS 		"\"flags\": "
#define SSID 		"\"ssid\": "

#define MAXLEN 4096

void wpa_scan();
/*
int main(int argc, char *argv[])
{
	wifi_info *head;
	char *json_buf;

	wpa_scan();
	head = parse_result(TEMP_PATH);
	print_info(head);
	printf("here\n");
	json_buf = info_json_encode(head);
	if(json_buf != NULL)
		printf("%s\n", json_buf);
	else 
		printf("null\n");
	free(json_buf);
	exit(0);

}
*/
void wpa_scan()
{
	char shell[256];
	system("wpa_cli -iwlan0 scan");
	sprintf (shell, "wpa_cli -iwlan0 scan_results > %s", TEMP_PATH);
	system(shell);
}

wifi_info *parse_result(char *path)
{
	FILE *fp;
	char buf[MAXLINE];
	wifi_info *info;
	wifi_info *head = NULL;
	char *token;
	int i, n, j;
	//int spec_count;  //json 特殊字符容度

	fp = fopen(path, "r+");
	if (fp == NULL) {
		perror("open error");
		return NULL;
	}
	fgets(buf, MAXLINE, fp);
	while (fgets(buf, MAXLINE, fp)) {
		i = 0;
		//spec_count = 10;
		if ((token = strtok(buf, "\t")) != NULL) {
			info = (wifi_info *)Malloc(sizeof(wifi_info));
			n = strlen(token) + 1;
			info->info_array[i] = (char *)Malloc(n);
			strncpy(info->info_array[i], token, n); 
			//printf("%d. %s\n",i, info->info_array[i]);

			for (i = 1; i < 5; i ++) {
				if ((token = strtok(NULL, "\t")) != NULL) {
					n = strlen(token) + 1;
					info->info_array[i] = (char *)Malloc(n * 2);		//提供两倍大小以防止json特殊过多字符串出现
					j = 0;
					while (*token != '\0') {
						if (*token == '[' || *token == ']' ||
								*token == '{' || *token == '}' ||
								*token == '"' ) {
							info->info_array[i][j++] = '\\';
							n++;
							//spec_count --;
						}
						info->info_array[i][j++] = *token++;
					}
					info->info_array[i][j] = '\0';
				}
				else {
					while (i-- >= 0) {
						free(info->info_array[i]);
					}
					free(info);
					break;			//break loop of for
				}
			}

			if (i == 5) {
				info->info_array[4][n-2] = '\0';		//将行末回车符号去掉
 				head = add_info(head, info);
			}
			/*while ((token = strtok(NULL, "\t")) != NULL) {
				printf("%d. %s\n", count+, token);
			}*/
		}
	}
	return head;
}

/*
* 
*/
wifi_info *add_info(wifi_info *head, wifi_info *info)
{
	if (head == NULL) {
		head = (wifi_info *)Malloc(sizeof(wifi_info));
		head = info;
		head->next = NULL;
	}
	else {
		head->next = add_info(head->next, info);
	}
	return head;
}

void free_info(wifi_info *head)
{
	int i;
	if (head != NULL) {
		free_info(head->next);
		for (i = 0; i < 5; i++) {
			free(head->info_array[i]);
		}
		free(head);
	}
}

void print_info(wifi_info *head)
{
	wifi_info *p;
	int i;
	for (p = head; p != NULL; p = p->next) {
		for (i = 0; i < 5; i++) {
			printf("%d. %s\n", i, p->info_array[i]);
		}
		printf("\n");
	}
}

/*
*
*/
char *info_json_encode(wifi_info *head)
{
	char *json_buf = NULL;
	char *obj = NULL;
	char *obj_set = NULL;
	wifi_info *p;
	int n = 0;
	int count = 0;
	size_t set_size = MAXLEN;

	if (head == NULL) return NULL;

	obj = (char *)Malloc(2048);
	obj_set = (char *)Malloc(set_size);
	obj_set[0] = '\0';

	count = sprintf(obj, "{\"bssid\": \"%s\", \"frenquency\": %s, \"signal_level\": %s, \"flags\": \"%s\", \"ssid\": \"%s\"}",
			head->info_array[0], head->info_array[1], head->info_array[2], head->info_array[3], head->info_array[4]);
	strncpy(obj_set, obj, count + 1);
	printf("1\n");

	for (p = head->next; p != NULL; p = p->next) {
		strcat(obj_set, ", ");
		count += 2;	
		n = sprintf(obj, "{\"bssid\": \"%s\", \"frenquency\": %s, \"signal_level\": %s, \"flags\": \"%s\", \"ssid\": \"%s\"}",
			p->info_array[0], p->info_array[1], p->info_array[2], p->info_array[3], p->info_array[4]);

		count += n;
		if (count >= set_size) {
			set_size += MAXLEN;
			obj_set = (char *)realloc(obj_set, set_size);
			if (obj_set == NULL) {
					perror("realloc error");
					goto exit;
			}
		}

		strcat(obj_set, obj);
	}
	printf("2\n");
	json_buf = (char *)Malloc(count + 30);
	printf("4 count: %d\n", count);
	sprintf(json_buf, "{\"scan_results\": [%s]}", obj_set);

exit:
	free(obj);
	free(obj_set);
	printf("3\n");
	return json_buf;
}

int add_wifi_connect_info()
{

}