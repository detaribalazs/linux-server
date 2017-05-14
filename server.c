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

int main(int argc, char** argv)
{
	int config_fd;
	char buf[CONFIG_PARAMETER_NUM*50];
	char default_cfg[49] = "IP_ADDR=127.0.0.1\nPORT_ADDR=8081\nMAX_CONN_NUM=5\n\0";
	int length;
	char ch = 0;
	int i = 0, j = 0, k = 0, pos = 0;
	char* cfg_data[CONFIG_PARAMETER_NUM];
	char ip_address[129];
	char port[5];
	int max_conn;
	char cfg_param_list[CONFIG_PARAMETER_NUM][CONFIG_PARAMETER_MAX_LENGTH] = {"IP_ADDR", "PORT_ADDR", "MAX_CONN_NUM"};
	char tmp[CONFIG_PARAMETER_MAX_LENGTH];
	char cfg_param_value[CONFIG_PARAMETER_NUM][CONFIG_PARAMETER_MAX_LENGTH];
	int value_flag = 0;

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
		else
		{
			close(config_fd);
			config_fd = open("./config.cfg", O_RDONLY);
			if(config_fd < 0)
			{
				printf("Couldn't create config.cfg");
				return 1;
			}	
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
	i = j = pos = 0;
	ch = 0;
	for(i=0; i<CONFIG_PARAMETER_NUM; i++)
	{
		value_flag = 0;
		for(j=0; j<strlen(cfg_data[i])+1; j++)
		{
			if(!value_flag)
			{
				tmp[j] = cfg_data[i][j];
				if(tmp[j] == '=')
				{
					tmp[j] = '\0';
					if(!strcmp(tmp, cfg_param_list[i]))
					{
						/* known config paramter, value starts */
						value_flag = 1;
						pos = j;
					}
					else
					{
						perror("Unknown config parameter.");
						return -1;	
					}
				}
			}
			else
			{
				tmp[j-pos-1] = cfg_data[i][j];
				/* read till the end of string */
				if(tmp[j-pos-1] == '\0')
				{
					/* find out is paramter numeric or string */
					if(strstr(cfg_param_list[i], "NUM"))
					{
						max_conn = strtoimax(tmp, NULL, 10);
					}
					else
					{
						if( (strcpy((char*)(&cfg_param_value[i]), (char*)tmp) == NULL) )
						{
							perror("String copy error");
						}
					}
				}
			}
		}
	}
	strcpy((char*)ip_address, (char*)(&cfg_param_value[0]));
	strcpy((char*)port, (char*)(&cfg_param_value[1]));
	printf("%s\n", ip_address);
	printf("%s\n", port);
	printf("%d\n", max_conn);
	return 0;
}