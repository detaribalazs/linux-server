#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>


#define CONFIG_PARAMETER_NUM 		3
#define CONFIG_PARAMETER_MAX_LENGTH	100

typedef struct{
	char ip_address[129];
	char port[5];
	int max_conn;
} config_parameter_t;

int configure_server(config_parameter_t* dest);