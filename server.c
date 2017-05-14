#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define CONFIG_PARAMETER_NUM 	3

int main(int argc, char** argv)
{
	int config_fd;
	char buf[CONFIG_PARAMETER_NUM*50];
	char default_cfg[48] = "IP_ADDR=127.0.0.1\nPORT_ADDR=8081\nMAX_CONN_NUM=5\0";
	int length;
	char ch = 'a';
	int i = 0, j = 0, k = 0, pos = 0;
	char* cfg_data[CONFIG_PARAMETER_NUM];
	char ip_address[129];
	char port[5];
	int max_conn;

	/* 	read config parameters */ 
	config_fd = open("./config.cfg", O_RDONLY);
	if( config_fd < 0)
	{
		perror("Missing config.cfg");
		config_fd = creat("./config.cfg", 0644);
		if(config_fd < 0)
		{
			printf("Couldn't create config.cfg");
			return 1;
		}	

		if(write(config_fd, default_cfg, strlen(default_cfg)) != strlen(default_cfg))
		{
			perror("Couldn't write default config parameters.");
			printf("%d\n", errno);
			return 1;
		}
	}

	length = read(config_fd, buf, sizeof(buf) - 1);
	if(length < 0)
	{
		perror("config_read");
		return 1;
	}
	printf("%s\n", buf);

	buf[length] = '\0';
	while(j != CONFIG_PARAMETER_NUM)
	{
		while(ch != '\n')
		{
			ch = buf[i + pos];
			i++;
		}
		cfg_data[j] = (char*) malloc(i);
		cfg_data[j][i-1] = '\0';
		i = i-2; 
		if(cfg_data[j] == NULL)
		{
			perror("Memory error.");
			return 1;
		}
		for(k=i; k>=0; k--)
		{
			cfg_data[j][k] = buf[pos + k];
		}
		pos = pos + i + 2;
		i = 0;
		j++;
		ch = 0;
	}
	
	return 0;
}