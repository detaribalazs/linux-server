#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>


#define CONFIG_PARAMETER_NUM 			5
#define CONFIG_PARAMETER_MAX_LENGTH		100
#define DEFAULT_CONFIG_STRING 			"LOCALHOST_NUM=1\nPORT_ADDR=8081\nMAX_CONN_NUM=5\nSS_PHP_NUM=1\nLOG_NUM=1\n\0";
#define CONFIG_PARAMETER_LIST			{"LOCALHOST_NUM", "PORT_ADDR", "MAX_CONN_NUM", "SS_PHP_NUM", "LOG_NUM"}

typedef struct{
	int localhost;
	char port[5];
	int max_conn;
	int ss_php;
	int log;
} config_parameter_t;

int configure_server(config_parameter_t* dest);