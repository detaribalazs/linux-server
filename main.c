#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/* configuration header */
#include "config.h"

/* tcp headers */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct{
	char portnum[6];
	int  used;
}port_t;

int init_ports(port_t* ports, char* start_port, int num)
{	
	int i;

	ports = (port_t*) malloc(num * sizeof(port_t));
	if(ports == NULL)
	{
		perror("Port init malloc error");
		return 1;
	}
	int start = strtoimax(start_port, NULL, 10);

	printf("ports used:\n");
	for(i=0; i<num; i++)
	{	
		sprintf(ports[i].portnum, "%d", start+i);
		ports[i].used = 0;
		#ifdef DEBUG
		printf("%s\n", ports[i].portnum);
		#endif
	}
	return 0;
}

int main(int argc, char** argv)
{
	/*********************** TCP server ******************/
	struct addrinfo hints;
  	struct addrinfo* res;
  	int err;
  	struct sockaddr_in6 address;
  	socklen_t addrlen;
  	char ips[NI_MAXHOST];
  	char servs[NI_MAXSERV];
  	int ssock, csock;
  	char buf[256];
  	int len;
  	int reuse;
  	/****************************************************/

	config_parameter_t server_parameters;
	memset(&server_parameters, 0, sizeof(server_parameters));
	port_t port_list;
	int sock;


	/************************TCP ***************************/

  	memset(&hints, 0, sizeof(hints));
  	hints.ai_flags = AI_PASSIVE;
  	hints.ai_family = AF_INET6;
  	hints.ai_socktype = SOCK_STREAM;
  	/***********************************************************/

	configure_server(&server_parameters);
	#ifdef DEBUG
	printf("%d\n", server_parameters.localhost);
	printf("%s\n", server_parameters.port);
	printf("%d\n", server_parameters.max_conn);
	printf("%d\n", server_parameters.ss_php);
	printf("%d\n", server_parameters.log);
	#endif

	if(init_ports(&port_list, server_parameters.port, server_parameters.max_conn))
	{
		return 1;
	}


	/* localhost is set */
	if(server_parameters.localhost)
	{
		address.sa_family_t = AF_INET6;
	}
	/* we need address info */
	else
	{
		printf("getaddr\n");
	}
	sock = socket(PF_INET, SOCK_STREAM, 0);


	return 0;

}