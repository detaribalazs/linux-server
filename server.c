#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include "config.h"

int main(int argc, char** argv)
{
	config_parameter_t server_parameters;
	memset(&server_parameters, 0, sizeof(server_parameters));

	configure_server(&server_parameters);
	printf("%s\n", server_parameters.ip_address);
}